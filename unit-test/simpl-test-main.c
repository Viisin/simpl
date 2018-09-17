/** SIMPL Is Memory Pool Library (SIMPL)
 *  @file      simpl-test.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "simpl.h"
#include "simpl-unit-test.h"
#include "simpl-unit-test-construction.c"
#include "simpl-unit-test-memalign.c"
#include "simpl-unit-test-realloc.c"
#include "simpl-unit-test-drain.c"
#include "simpl-unit-test-destruction.c"

#define TEST(func, mempool) \
	do { \
		printf("  %s: ", #func); \
		printf("%s\n", func? (func(mempool)? \
			"FAILED": "SUCCEED"): "UNKNOWN FUNCTION"); \
	} while(0)
int main(int argc, char *argv[])
{
	struct mempool simpl = {
		.buffer_size = 1U << 30 /* 1GB */, 
		.buffer = NULL,
		.init = simpl_init,
		.malloc = simpl_malloc,
		.free = simpl_free,
		.realloc = simpl_realloc,
		.memalign = simpl_memalign,
		.dump = NULL,
		.handle = NULL,
		.pool_overhead = 0, /* not support */
		.alloc_overhead = 0 /* not support */
	};

	printf("[Mempool Test]\n");
	TEST(construction_test, &simpl);
	TEST(memalign_test, &simpl);
	TEST(realloc_test, &simpl);
	TEST(drain_test, &simpl);
	TEST(destruction_test, &simpl);
	printf("Finished!\n");

	return 0;
}
