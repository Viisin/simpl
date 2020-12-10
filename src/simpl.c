/** SIMPL Is Memory Pool Library (SIMPL)
 *  @file      simpl.c
 *  @author    Ozpin Lin <c20viisin@gmail.com>
 *  @copyright Copyright (c) 2018, Ozpin Lin
 *  @details
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice,
 *     and the entire permission notice in its entirety,
 *     including the disclaimer of warranties.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be inuse to endorse or promote products
 *     derived from this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *  FITNESS FOR A PARTICULAR PURPOSE, ALL OF WHICH ARE HEREBY DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "simpl.h"

#ifndef assert_msg
#if (defined(_DEBUG) && !defined(NDEBUG))
#include <stdio.h>
#include <assert.h>

#define assert_msg(cond, fmt, ...) \
	do { \
		if (!(cond)) { \
			printf("\n%s(%d): assertion failed(%s)\n", \
				__FILE__, __LINE__, #cond); \
			if (fmt) { \
				printf("%s: ", __func__); \
				printf(fmt, ##__VA_ARGS__); \
				printf("\n"); \
			} \
			assert(cond); \
		} \
	} while(0)
#else
#define assert_msg(cond, fmt, ...)
#endif//(_DEBUG && !NDEBUG)
#endif//assert_msg

#ifndef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif//offsetof

#ifndef container_of
#define container_of(ptr, type, member) ((type *)((uintptr_t)(ptr) - offsetof(type, member)))
#endif//container_of

#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) && defined(__GNUC_PATCHLEVEL__) /* GCC 3.4 and above */
static inline int ffs(uint32_t dw) {
	return dw? __builtin_ffs(dw): 0;
}

static inline int fls(uint32_t dw) {
	return dw? 32 - __builtin_clz(dw): 0;
}
#elif defined(_MSC_VER) && (_MSC_VER >= 1400) && (defined(_M_IX86) || defined(_M_X64)) /* VS x86/x64 */
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

static inline int ffs(uint32_t dw) {
	unsigned long index;
	return _BitScanForward(&index, dw)? index + 1: 0;
}

static inline int fls(uint32_t dw) {
	unsigned long index;
	return _BitScanReverse(&index, dw)? index + 1: 0;
}
#else
static inline int fls_generic(uint32_t dw) {
	int bit = 32;

	if (!dw) bit -= 1;
	if (!(dw & 0xffff0000)) { dw <<= 16; bit -= 16; }
	if (!(dw & 0xff000000)) { dw <<= 8; bit -= 8; }
	if (!(dw & 0xf0000000)) { dw <<= 4; bit -= 4; }
	if (!(dw & 0xc0000000)) { dw <<= 2; bit -= 2; }
	if (!(dw & 0x80000000)) { dw <<= 1; bit -= 1; }

	return bit;
}

static inline int ffs(uint32_t dw) { 
	return fls_generic(dw & (~dw + 1));
}

static inline int fls(uint32_t dw) {
	return fls_generic(dw);
}
#endif

static inline size_t align_up(size_t val, size_t align) {
	size_t mask = align - 1;
	return val + mask & ~mask;
}

static inline void *ptr_align_up(void *ptr, size_t align) {
	uintptr_t mask = align - 1;
	return (void *)((uintptr_t)ptr + mask & ~mask);
}

static inline void *ptr_align_down(void *ptr, size_t align) {
	return (void *)((uintptr_t)ptr & ~((uintptr_t)align - 1));
}

#if (defined(_DEBUG) && !defined(NDEBUG))
static inline int is_aligned(size_t val, size_t align) {
	return ((val & align - 1) == 0);
}

static inline int is_ptr_aligned(void *ptr, size_t align) {
	return (((uintptr_t)ptr & (uintptr_t)align - 1) == 0);
}
#endif//(_DEBUG && !NDEBUG)

/** <pre>
 *  +---------[CHUNK]---------+
 *  | Physical Previous Chunk |\
 *  +---------------------+-+-+ \
 *  |                Size |P|F|}-\------(USED)
 *  +~~~~~~~~[PAYLOAD]~~~~~~~~+   (FREE)
 *  |           Free Previous |  /
 *  +-------------------------+ /
 *  |               Free Next |/
 *  +-------------------------+ </pre> */
struct simpl_chunk {
	/** not allowed to access when previous physical chunk used */
	struct simpl_chunk *phys_prev;
	/** <pre>
	 *  chunk size first bit:  chuck free flag
	 *  chunk size second bit: previous physical chunk free flag
	 *  chunk size must 4 bytes aligned </pre> */
	uint32_t size;
#define chunk_flag_free_mask      (0x1U)
#define chunk_flag_prev_free_mask (0x2U)
#define chunk_flags_mask          (0x3U)
#define is_chunk_free(chunk)      ((chunk)->size & chunk_flag_free_mask)   
#define is_chunk_prev_free(chunk) ((chunk)->size & chunk_flag_prev_free_mask)   
#define get_chunk_flags(chunk)    ((chunk)->size & chunk_flags_mask)   
#define get_chunk_size(chunk)     ((chunk)->size & ~chunk_flags_mask)
	union {
		/** C++ not allow zero-sized array */
		uint8_t *payload[1];
		/** "not allowed to access when chunk used */
		struct {
			struct simpl_chunk *free_prev;
			struct simpl_chunk *free_next;
		};
	};
};

/** <pre>
 *  |------------------------[BITMAP]------------------------| (index: 0 ~ 191, 1G: 0 ~ 175)
 *  |23|2048M|2304M|2560M|2816M|3072M|3328M|3584M|3840M|+256M| 1XXX .... .... .... .... .... .... ..00
 *  |22|1024M|1152M|1280M|1408M|1536M|1664M|1792M|1920M|+128M| 01XX X... .... .... .... .... .... ..00
 *  |21| 512M| 576M| 640M| 704M| 768M| 832M| 896M| 960M| +64M| 001X XX.. .... .... .... .... .... ..00
 *  |20| 256M| 288M| 320M| 352M| 384M| 416M| 448M| 480M| +32M| 0001 XXX. .... .... .... .... .... ..00
 *  |19| 128M| 144M| 160M| 176M| 192M| 208M| 224M| 240M| +16M| 0000 1XXX .... .... .... .... .... ..00
 *  |18|  64M|  72M|  80M|  88M|  96M| 104M| 112M| 120M|  +8M| 0000 01XX X... .... .... .... .... ..00
 *  |17|  32M|  36M|  40M|  44M|  48M|  52M|  56M|  60M|  +4M| 0000 001X XX.. .... .... .... .... ..00
 *  |16|xxxxx|   4M|   8M|  12M|  16M|  20M|  24M|  28M|  +4M| 0000 000X XX.. .... .... .... .... ..00
 *  |--|-----------------------------------------------|-----|
 *  |15|2048K|2304K|2560K|2816K|3072K|3328K|3584K|3840K|+256K| 0000 0000 001X XX.. .... .... .... ..00
 *  |14|1024K|1152K|1280K|1408K|1536K|1664K|1792K|1920K|+128K| 0000 0000 0001 XXX. .... .... .... ..00
 *  |13| 512K| 576K| 640K| 704K| 768K| 832K| 896K| 960K| +64K| 0000 0000 0000 1XXX .... .... .... ..00
 *  |12| 256K| 288K| 320K| 352K| 384K| 416K| 448K| 480K| +32K| 0000 0000 0000 01XX X... .... .... ..00
 *  |11| 128K| 144K| 160K| 176K| 192K| 208K| 224K| 240K| +16K| 0000 0000 0000 001X XX.. .... .... ..00
 *  |10|  64K|  72K|  80K|  88K|  96K| 104K| 112K| 120K|  +8K| 0000 0000 0000 0001 XXX. .... .... ..00
 *  | 9|  32K|  36K|  40K|  44K|  48K|  52K|  56K|  60K|  +4K| 0000 0000 0000 0000 1XXX .... .... ..00
 *  | 8|xxxxx|   4K|   8K|  12K|  16K|  20K|  24K|  28K|  +4K| 0000 0000 0000 0000 0XXX .... .... ..00
 *  |--|-----------------------------------------------|-----|
 *  | 7|2048 |2304 |2560 |2816 |3072 |3328 |3584 |3840 |+256 | 0000 0000 0000 0000 0000 1XXX .... ..00
 *  | 6|1024 |1152 |1280 |1408 |1536 |1664 |1792 |1920 |+128 | 0000 0000 0000 0000 0000 01XX X... ..00
 *  | 5| 512 | 576 | 640 | 704 | 768 | 832 | 896 | 960 | +64 | 0000 0000 0000 0000 0000 001X XX.. ..00
 *  | 4| 256 | 288 | 320 | 352 | 384 | 416 | 448 | 480 | +32 | 0000 0000 0000 0000 0000 0001 XXX. ..00
 *  | 3| 128 | 144 | 160 | 176 | 192 | 208 | 224 | 240 | +16 | 0000 0000 0000 0000 0000 0000 1XXX ..00
 *  | 2|  64 |  72 |  80 |  88 |  96 | 104 | 112 | 120 |  +8 | 0000 0000 0000 0000 0000 0000 01XX X.00
 *  | 1|  32 |  36 |  40 |  44 |  48 |  52 |  56 |  60 |  +4 | 0000 0000 0000 0000 0000 0000 001X XX00
 *  | 0|xxxxxxxxxxx|   8 |  12 |  16 |  20 |  24 |  28 |  +4 | 0000 0000 0000 0000 0000 0000 000X XX00
 *  |--|-----|-----|-----|-----|-----|-----|-----|-----|-----|
 *  |      0 |   1 |   2 |   3 |   4 |   5 |   6 |   7       |
 *  |--------------------------------------------------------| </pre> */
struct simpl_pool {
	uint32_t available;
	uint32_t fl_bitmap;
	uint8_t *sl_bitmaps;
	struct simpl_chunk **freelists;
#define simplc_fl_shift              (0x3)
#define simplc_sl_mask               (0x7)
#define get_fl_index(fi)             ((fi) >> simplc_fl_shift)
#define get_sl_index(fi)             ((fi) & simplc_sl_mask)
#define get_freelist_index(fli, sli) (((fli) << simplc_fl_shift) | (sli))
};

enum simpl_const {
	simplc_bytes_per_ptr = sizeof(uintptr_t),
	simplc_bits_per_byte = 8,

	simplc_4B_shift      = 2,
	simplc_4kB_shift     = 12,
	simplc_4MB_shift     = 22,
	simplc_4kB_size      = 1U << simplc_4kB_shift,
	simplc_4MB_size      = 1U << simplc_4MB_shift,
	simplc_max_flsize    = 24,
	simplc_max_freelists = simplc_max_flsize * simplc_bits_per_byte,

	simplc_chunk_overlap_size = offsetof(struct simpl_chunk, size),
	simplc_chunk_overhead     = offsetof(struct simpl_chunk, payload) - simplc_chunk_overlap_size,
	simplc_chunk_min_size     = sizeof(struct simpl_chunk) - simplc_chunk_overhead,
#define simplc_chunk_max_size (UINT32_MAX)
};

static inline uint32_t adjust_alloc_size(size_t alloc_size, size_t align) {
	uint32_t adj_size;

	if (alloc_size > simplc_chunk_max_size)
		return 0;
	adj_size = (uint32_t)alloc_size;
	if (adj_size < simplc_chunk_min_size)
		adj_size = simplc_chunk_min_size;
	adj_size = (uint32_t)align_up(adj_size, align);
	if (adj_size < alloc_size) /* overflow */
		return 0;
	return adj_size;
}

static inline void set_chunk_size(struct simpl_chunk *chunk, uint32_t size) {
	assert_msg(!(size & chunk_flags_mask), "size(%d) invalid.", size);
	chunk->size = size | get_chunk_flags(chunk);
}

static inline struct simpl_chunk *prev_phys_chunk(struct simpl_chunk *chunk) {
	assert_msg(is_chunk_prev_free(chunk), "chunk must prev_freed.");
	return chunk->phys_prev;
}

static inline struct simpl_chunk *next_phys_chunk(struct simpl_chunk *chunk) {
	return (struct simpl_chunk *)((uint8_t *)chunk + simplc_chunk_overhead + get_chunk_size(chunk));
}

static inline void set_chunk_free(struct simpl_chunk *chunk) {
	chunk->size |= chunk_flag_free_mask;
	next_phys_chunk(chunk)->size |= chunk_flag_prev_free_mask;
}

static inline void set_chunk_used(struct simpl_chunk *chunk) {
	chunk->size &= ~chunk_flag_free_mask;
	next_phys_chunk(chunk)->size &= ~chunk_flag_prev_free_mask;
}

static inline void *get_chunk_payload(struct simpl_chunk *chunk) {
	return &chunk->payload;
}

static inline struct simpl_chunk *get_payload_chunk(void *payload) {
	return container_of(payload, struct simpl_chunk, payload);
}

/** @brief    Get size of freelists mapping.
 *  @param fi The size and freelists index mapping.
 *  @return   The size of mapping */
static uint32_t mapping_size(uint32_t fi)
{
	uint32_t size, size_shift, fli_local, fli = get_fl_index(fi);

	if (fli < 8) {
		fli_local = fli;
		size_shift = 0;
	} else if (fli < 16) {
		fli_local = fli - 8;
		size_shift = 10;
	} else {
		fli_local = fli - 16;
		size_shift = 20;
	}

	size = fli_local? 32 << (fli_local - 1): 0;
	size += get_sl_index(fi) * (size? size >> 3: 4);
	return size <<= size_shift;
}

/** @brief      Size and freelists index mapping.
 *  @param pool Pool header.
 *  @param size Adjusted chunk size.
 *  @return     The size and freelists index mapping.
 *  @note       size can't over UINT32_MAX. */
static uint32_t freelists_mapping(uint32_t size)
{
	uint32_t fli, sli;
	int ls;

	if (size < simplc_4kB_size) {
		fli = 0;
		size >>= simplc_4B_shift;
	} else if (size < simplc_4MB_size) {
		fli = 8;
		size >>= simplc_4kB_shift;
	} else {
		fli = 16;
		size >>= simplc_4MB_shift;
	}

	ls = fls(size);
	if (ls > 3) {
		fli += ls - 3;
		sli = (size >> (ls - 4)) & simplc_sl_mask;
	} else {
		sli = size & simplc_sl_mask;
	}

	return get_freelist_index(fli, sli);
}

static inline uint32_t size_roundup(uint32_t size)
{
	uint32_t fi = freelists_mapping(size);
	return size > mapping_size(fi)? mapping_size(fi + 1): size;
}

static inline void set_bitmap(struct simpl_pool *pool, uint32_t fi) {
	uint32_t fli = get_fl_index(fi);
	pool->fl_bitmap |= 1U << fli;
	pool->sl_bitmaps[fli] |= 1U << get_sl_index(fi);
}

/** @brief           Push free chunk into freelists
 *  @param[in] pool  Pool header.
 *  @param[in] chunk The free chunk which need to push into freelists. */
static void push_free_chunk(struct simpl_pool *pool, struct simpl_chunk *chunk)
{
	uint32_t chunk_size = get_chunk_size(chunk);
	uint32_t fi = freelists_mapping(chunk_size);
	struct simpl_chunk *head = pool->freelists[fi];

	assert_msg(is_chunk_free(chunk), "chunk must freed.");
	if (head)
		head->free_prev = chunk;
	chunk->free_prev = NULL;
	chunk->free_next = head;
	pool->freelists[fi] = chunk;
	set_bitmap(pool, fi);

	pool->available += chunk_size;
}

static inline void clr_bitmap(struct simpl_pool *pool, uint32_t fi) {
	uint32_t fli = get_fl_index(fi);
	pool->sl_bitmaps[fli] &= ~(1U << get_sl_index(fi));
	if (!pool->sl_bitmaps[fli])
		pool->fl_bitmap &= ~(1U << fli);
}

/** @brief           Pop free chunk from freelists
 *  @param[in] pool  Pool header.
 *  @param[in] chunk The free chunk which need to pop from freelists. */
static void pop_free_chunk(struct simpl_pool *pool, struct simpl_chunk *chunk)
{
	uint32_t chunk_size = get_chunk_size(chunk);
	uint32_t fi = freelists_mapping(chunk_size);
	struct simpl_chunk *prev = chunk->free_prev;
	struct simpl_chunk *next = chunk->free_next;

	assert_msg(is_chunk_free(chunk), "chunk must freed.");
	if (prev)
		prev->free_next = next;
	else
		pool->freelists[fi] = next;
	if (next)
		next->free_prev = prev;
	else
		clr_bitmap(pool, fi);

	pool->available -= chunk_size;
}

void *simpl_init(void *buffer, size_t buffer_size)
{
	const uint8_t *end = (uint8_t *)ptr_align_down((uint8_t *)buffer + buffer_size, simplc_bytes_per_ptr);
	struct simpl_pool *pool;
	struct simpl_chunk *chunk;
	uint8_t *p;
	uint32_t i, est, sl_size, size;

	if (!buffer || !buffer_size || (buffer_size > simplc_chunk_max_size))
		return NULL;
	p = (uint8_t *)ptr_align_up(buffer, simplc_bytes_per_ptr);
	pool = (struct simpl_pool *)p;

	p = p + sizeof(struct simpl_pool);
	pool->sl_bitmaps = p;

	est = freelists_mapping((uint32_t)(end - p)) + 1;
	sl_size = (est + simplc_bits_per_byte - 1) / simplc_bits_per_byte;
	assert_msg(est <= simplc_max_freelists,
		"est(%d) should not greater than const(%d).", est, simplc_max_freelists);
	assert_msg(sl_size <= simplc_max_flsize,
		"sl_size(%d) should not greater than const(%d).", sl_size, simplc_max_flsize);
	p = (uint8_t *)ptr_align_up(p + sl_size, simplc_bytes_per_ptr);
	pool->freelists = (struct simpl_chunk **)p;

	p = (uint8_t *)ptr_align_up(p + est * simplc_bytes_per_ptr, simplc_bytes_per_ptr);
	if (p > end)
		return NULL;
	size = (uint32_t)(end - p);
	if (size < simplc_chunk_overhead * 2 + simplc_chunk_min_size)
		return NULL;
	pool->available = 0;
	pool->fl_bitmap = 0;
	for (i = 0; i < sl_size; i++)
		pool->sl_bitmaps[i] = 0;
	for (i = 0; i < est; i++)
		pool->freelists[i] = NULL;

	chunk = (struct simpl_chunk *)(p - simplc_chunk_overlap_size);
	chunk->size = size - simplc_chunk_overhead * 2; /* always prev used */
	assert_msg(!is_chunk_prev_free(chunk),
		"first chunk must always prev used");
	next_phys_chunk(chunk)->size = 0; /* tail always used, and don't care phy_prev */
	set_chunk_free(chunk);
	push_free_chunk(pool, chunk);
	return pool;
}

/** @brief          Search available chunk from freelists.
 *  @param[in] pool Pool header.
 *  @param[in] size Adjusted chunk size which be required.
 *  @return         Freelists index.
 *  @note
 *  \p size can't over UINT32_MAX. */
static uint32_t search_freelists(struct simpl_pool *pool, uint32_t size)
{
	uint32_t fi, fli, sli;
	int fs;

	fi = freelists_mapping(size);
	fli = get_fl_index(fi);
	sli = get_sl_index(fi);

	fs = ffs(pool->sl_bitmaps[fli] & (~0U << sli));
	if (fs) {
		sli = fs - 1;
	} else {
		fs = ffs(pool->fl_bitmap & (~0U << (fli + 1)));
		if (!fs) /* not found */
			return 0;
		fli = fs - 1;
		sli = ffs(pool->sl_bitmaps[fli]) - 1;
	}
	fi = get_freelist_index(fli, sli);

	assert_msg(fi, "fi(%d) must not zero", fi);
	assert_msg(pool->freelists[fi],
		"freelists[%d] must exist.", fi);
	assert_msg(sli < simplc_bits_per_byte,
		"sli(%d) must smaller than const(%d)", sli, simplc_bits_per_byte);
	return fi;
}

/** @brief           Merge free neighbor chunk.
 *  @param[in] pool  Pool header.
 *  @param[in] chunk The chunk which need to merge free neighbor.
 *  @return          New chunk position. */
static struct simpl_chunk *merge_free_neighbor_chunk(struct simpl_pool *pool, struct simpl_chunk *chunk)
{
	uint32_t chunk_size;
	struct simpl_chunk *neighbor;

	assert_msg(is_chunk_free(chunk), "chunk must freed.");
	if (is_chunk_prev_free(chunk)) { /* merge prev chunk */
		neighbor = prev_phys_chunk(chunk);
		assert_msg(is_chunk_free(neighbor),
			"chunk prev_freed then prev chunk must freed.");
		pop_free_chunk(pool, neighbor);
		next_phys_chunk(chunk)->phys_prev = neighbor;
		
		chunk_size = get_chunk_size(neighbor) + simplc_chunk_overhead + get_chunk_size(chunk);
		set_chunk_size(neighbor, chunk_size);
		
		chunk = neighbor;
	}

	neighbor = next_phys_chunk(chunk);
	assert_msg(is_chunk_prev_free(neighbor),
		"chunk freed then next chunk must prev_freed.");
	if (is_chunk_free(neighbor)) { /* merge next chunk */
		pop_free_chunk(pool, neighbor);
		next_phys_chunk(neighbor)->phys_prev = chunk;
		
		chunk_size = get_chunk_size(chunk) + simplc_chunk_overhead + get_chunk_size(neighbor);
		set_chunk_size(chunk, chunk_size);
	}
	return chunk;
}

/** @brief               Trim head chunk and push exceed chunk to freelists.
 *  @param[in] pool      Pool header.
 *  @param[in] chunk     The chunk which need to trim.
 *  @param[in] trim_size Adjusted chunk size which be required.
 *  @return              The chunk which to use.
 *  @note
 *  \p trim_size can't over UINT32_MAX. */
static struct simpl_chunk *trim_chunk_to_use(struct simpl_pool *pool, struct simpl_chunk *chunk, uint32_t trim_size)
{
	struct simpl_chunk *trim;
	uint32_t chunk_size, remain;

	chunk_size = get_chunk_size(chunk);
	assert_msg(is_aligned(trim_size, simplc_bytes_per_ptr),
		"trim_size(%d) must %d bytes aligned", trim_size, simplc_bytes_per_ptr);
	assert_msg(trim_size <= chunk_size,
		"trim_size(%d) must smaller than chunk_size(%d).", trim_size, chunk_size);
	remain = chunk_size - trim_size;
	if (remain >= simplc_chunk_overhead + simplc_chunk_min_size) {
		chunk_size -= remain;
		set_chunk_size(chunk, chunk_size);

		trim = next_phys_chunk(chunk);
		trim->size = remain - simplc_chunk_overhead;
		next_phys_chunk(trim)->phys_prev = trim;

		set_chunk_used(chunk);
		set_chunk_free(trim);
		
		trim = merge_free_neighbor_chunk(pool, trim);
		push_free_chunk(pool, trim);
	} else {
		set_chunk_used(chunk);
	}
	return chunk;
}

void *simpl_malloc(void *simp, size_t alloc_size)
{
	struct simpl_pool *pool;
	struct simpl_chunk *chunk;
	uint32_t adj_size, fi;

	if (!simp || !alloc_size)
		return NULL;
	pool = (struct simpl_pool *)simp;

	adj_size = adjust_alloc_size(alloc_size, simplc_bytes_per_ptr);
	if (!adj_size || adj_size > pool->available)
		return NULL;
	adj_size = size_roundup(adj_size);
	if (!(fi = search_freelists(pool, adj_size)))
		return NULL;
	chunk = pool->freelists[fi];
	pop_free_chunk(pool, chunk);

	chunk = trim_chunk_to_use(pool, chunk, adj_size);
	return get_chunk_payload(chunk);
}

void simpl_free(void *simp, void *simple)
{
	struct simpl_pool *pool;
	struct simpl_chunk *chunk;

	if (!simp || !simple)
		return;
	pool = (struct simpl_pool *)simp;
	chunk = get_payload_chunk(simple);
	set_chunk_free(chunk);
	next_phys_chunk(chunk)->phys_prev = chunk;

	chunk = merge_free_neighbor_chunk(pool, chunk);
	push_free_chunk(pool, chunk);
}

void *simpl_realloc(void *simp, void *simple, size_t realloc_size)
{
	struct simpl_pool *pool;
	struct simpl_chunk *chunk, *prev, *next;
	uint32_t chunk_size, adj_size;
	void* payload;

	if (!simple)
		return simpl_malloc(simp, realloc_size);
	if (!simp || !realloc_size)
		return NULL;
	adj_size = adjust_alloc_size(realloc_size, simplc_bytes_per_ptr);
	if (!adj_size)
		return NULL;
	adj_size = size_roundup(adj_size);
	pool = (struct simpl_pool *)simp;
	chunk = get_payload_chunk(simple);
	chunk_size = get_chunk_size(chunk);

	if (adj_size <= chunk_size) { /* allow reduce size */
		chunk = trim_chunk_to_use(pool, chunk, adj_size);
		return get_chunk_payload(chunk);
	}

	next = next_phys_chunk(chunk);
	if (is_chunk_free(next)) { /* allow expand with next */
		chunk_size += simplc_chunk_overhead + get_chunk_size(next);
		if (adj_size <= chunk_size) {
			pop_free_chunk(pool, next);
			set_chunk_size(chunk, chunk_size);

			chunk = trim_chunk_to_use(pool, chunk, adj_size);
			return get_chunk_payload(chunk);
		}
	}
	if (is_chunk_prev_free(chunk)) { /* allow expand with prev, must memory move */
		prev = prev_phys_chunk(chunk);
		assert_msg(is_chunk_free(prev),
			"chunk prev_freed then prev chunk must freed.");
		chunk_size += get_chunk_size(prev) + simplc_chunk_overhead;
		if (adj_size <= chunk_size) {
			pop_free_chunk(pool, prev);
			if (is_chunk_free(next))
				pop_free_chunk(pool, next);
			set_chunk_size(prev, chunk_size);
			memmove(get_chunk_payload(prev), get_chunk_payload(chunk), get_chunk_size(chunk));

			chunk = trim_chunk_to_use(pool, prev, adj_size);
			return get_chunk_payload(chunk);
		}
	}

	payload = simpl_malloc(simp, adj_size);  /* find other chunk, must memory copy */
	if (payload) { 
		memcpy(payload, simple, get_chunk_size(chunk));
		simpl_free(simp, simple);
	}		
	return payload;
}

void *simpl_memalign(void *simp, size_t align, size_t alloc_size)
{
	struct simpl_pool *pool;
	struct simpl_chunk *chunk, *aligned_chunk;
	size_t mask;
	uint32_t adj_size, fi, chunk_size, size;
	uint8_t *p, *q;

	if (align < simplc_bytes_per_ptr)
		align = simplc_bytes_per_ptr;
	mask = align - 1;
	if (!simp || !alloc_size || align & mask || alloc_size & mask)
		return NULL;
	pool = (struct simpl_pool *)simp;

	adj_size = adjust_alloc_size(alloc_size, align);
	if (!adj_size || adj_size > pool->available)
		return NULL;
	adj_size = size_roundup(adj_size);
	if (!(fi = search_freelists(pool, simplc_chunk_min_size + (uint32_t)align + adj_size)))
		return NULL;
	chunk = pool->freelists[fi];
	pop_free_chunk(pool, chunk);

	chunk_size = get_chunk_size(chunk);
	p = (uint8_t *)get_chunk_payload(chunk);
	q = (uint8_t *)ptr_align_up(p, align);
	aligned_chunk = get_payload_chunk(q);

	if (q == p) {
		set_chunk_size(aligned_chunk, chunk_size);
	} else {
		size = (uint32_t)(q - p) - simplc_chunk_overhead;
		set_chunk_size(chunk, size);
		set_chunk_free(chunk);
		push_free_chunk(pool, chunk);
		
		aligned_chunk->phys_prev = chunk;
		set_chunk_size(aligned_chunk, chunk_size - size - simplc_chunk_overhead);
	}
	aligned_chunk = trim_chunk_to_use(pool, aligned_chunk, adj_size);
	return get_chunk_payload(aligned_chunk);
}
