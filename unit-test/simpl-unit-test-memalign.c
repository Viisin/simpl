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

int memalign_test(struct mempool *m)
{
	const size_t align = 1 << 10;
	void *p;

	if (!m->handle || !m->free || !m->realloc)
		return -EFAULT;
	p = m->memalign(m->handle, align, align * 4);
	if (!p)
		return -ENOMEM;
	if ((uintptr_t)p & (align - 1)) {
		m->free(m->handle, p);
		return -EFAULT;	}
	m->free(m->handle, p);
	return 0;
}
