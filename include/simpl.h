/** SIMPL Is Memory Pool Library (SIMPL)
 *  @file      simpl.h
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
#ifndef _SIMPL_H
#define _SIMPL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief                 Initialize memory buffer to SIMP.
 *  @param[in] buffer      Memory buffer for initialize.
 *  @param[in] buffer_size The Memory buffer size.
 *  @return                SIMP handle.
 *  @note
 *  \p buffer_size can't over UINT32_MAX. */
void *simpl_init(void *buffer, size_t buffer_size);

/** @brief                Allocate element from SIMP.
 *  @param[in] simp       SIMP handle.
 *  @param[in] alloc_size Allocated memory size.
 *  @return               SIMPL element.
 *  @note
 *  1. No lock implementation.
 *  2. \p alloc_size can't over UINT32_MAX. */
void *simpl_malloc(void *simp, size_t alloc_size);

/** @brief            Free SIMP element.
 *  @param[in] simp   SIMP handle.
 *  @param[in] simple SIMPL element.
 *  @note
 *  No lock implementation. */
void simpl_free(void *simp, void *simple);

/** @brief                  Reallocate element from SIMP.
 *  @param[in] simp         SIMP handle.
 *  @param[in] simple       SIMPL element.
 *  @param[in] realloc_size Reallocated memory size.
 *  @return                  SIMPL element.
 *  @note
 *  1. No lock implementation.
 *  2. \p realloc_size can't over UINT32_MAX. */
void *simpl_realloc(void *simp, void *simple, size_t realloc_size);

/** @brief                Allocate aligned element from SIMP.
 *  @param[in] simp       SIMP handle.
 *  @param[in] align      Aligned size
 *  @param[in] alloc_size Allocated memory size.
 *  @return               SIMPL element.
 *  @note
 *  1. No lock implementation.
 *  2. \p alloc_size can't over UINT32_MAX. */
void *simpl_memalign(void *simp, size_t align, size_t alloc_size);

#ifdef __cplusplus
};
#endif

#endif//_SIMPL_H
