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

int realloc_test(struct mempool *m)
{
	enum memory_pos {
		pos_compare,
		pos_head,
		pos_middle,
		pos_tail,
		num_of_pos
	};

	void *mem[num_of_pos], *p;
	size_t size;
	int i, r = 0;

	if (!m->handle || !m->free || !m->realloc)
		return -EFAULT;
	size = (m->buffer_size - (m->pool_overhead + (m->alloc_overhead * num_of_pos))) / (num_of_pos + 1);
	for (i = 0; i < num_of_pos; i++) {
		mem[i] = m->realloc(m->handle, NULL, size);
		if (!mem[i]) {
			while (i--)
				m->free(m->handle, mem[i]);
			return -ENOMEM;
		}
		if (i != pos_compare)
			memset(mem[i], i, size);
	}
	memcpy(mem[pos_compare], mem[pos_middle], size);

	p = m->realloc(m->handle, mem[pos_tail], size / 2); /* reduce size */
	if (!p) {
		for (i = 0; i < num_of_pos; i++)
			m->free(m->handle, mem[i]);
		return -EFAULT;
	}
	
	m->free(m->handle, mem[pos_tail]);
	p = m->realloc(m->handle, mem[pos_middle], size * 2);
	if (!p) {
		for (i = 0; i <= pos_middle; i++)
			m->free(m->handle, mem[i]);
		return -ENOMEM;
	}
	if (p != mem[pos_middle] || memcmp(p, mem[pos_compare], size)) {
		for (i = 0; i < pos_middle; i++)
			m->free(m->handle, mem[i]);
		m->free(m->handle, p);
		return -EFAULT;
	}

	m->free(m->handle, mem[pos_head]);
	p = m->realloc(m->handle, mem[pos_middle], size * 3);
	if (!p) {
		m->free(m->handle, mem[pos_compare]);
		m->free(m->handle, mem[pos_middle]);
		return -ENOMEM;
	}
	if (p != mem[pos_head] || memcmp(p, mem[pos_compare], size)) {
		m->free(m->handle, mem[pos_compare]);
		m->free(m->handle, p);
		return -EFAULT;
	}

	m->free(m->handle, p);
	m->free(m->handle, mem[pos_compare]);
	return 0;
}
