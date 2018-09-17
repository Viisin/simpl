/** SIMPL Is Memory Pool Library (SIMPL)
 *  @file simpl-gtest.c
 *  @author Ozpin Lin <c20viisin@gmail.com>
 *  @copyright Copyright (c) 2018, Ozpin Lin
 *  @section LICENSE
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
#include <errno.h>
#include "simpl-unit-test.h"

int drain_test(struct mempool *m)
{
	void **mem;
	size_t i, j, est, mem_count, overhead_buffer;

	enum drain_object_size {
		drian_tiny,
		drain_small,
		drain_middle,
		drain_large,
		drain_huge,
		num_of_drain
	};

	const size_t drain_object[num_of_drain] = {8U, 32U, 256U, 3 * 1024U, 3 * 1024U * 1024U};

	if (!m->handle || !m->malloc || !m->free)
		return -EFAULT;
	for (i = drian_tiny; i < num_of_drain; i++) {
		est = (m->buffer_size - m->pool_overhead) / (drain_object[i] + m->alloc_overhead);
		if (est == 0)
			continue;
		overhead_buffer = sizeof(void *) * est;
		mem = (void **)m->malloc(m->handle, overhead_buffer);
		if (!mem)
			return -ENOMEM;
		mem_count = (m->buffer_size - m->pool_overhead - overhead_buffer) / (drain_object[i] + m->alloc_overhead);
		for (j = 0; j < mem_count; j++) {
			mem[j] = m->malloc(m->handle, drain_object[i]);
			if (!mem[j])
				break;
			memset(&((uint8_t *)mem[j])[drain_object[i] - sizeof(uint32_t)], -1, sizeof(uint32_t)); /* destory modify last dwrod */
		}
		mem_count = j;
		for (j = 0; j < mem_count; j++)
			m->free(m->handle, mem[j]);
		m->free(m->handle, mem);
	}
	return 0;
}
