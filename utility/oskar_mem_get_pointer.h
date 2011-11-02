/*
 * Copyright (c) 2011, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSKAR_MEM_GET_POINTER_H_
#define OSKAR_MEM_GET_POINTER_H_

/**
 * @file oskar_mem_get_pointer.h
 */

#include "oskar_global.h"
#include "utility/oskar_Mem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns oskar_Mem structure which holds a pointer to memory held
 * within another oskar_Mem structure.
 *
 * @details
 * Note: The returned memory structure is set to not hold ownership of
 * memory which it points to.
 *
 * @param[out] ref         oskar_Mem structure holding a pointer to a location
 *                         in memory and its associated meta-data.
 * @param[in] mem          oskar_Mem structure from which the pointer reference
 *                         is made.
 * @param[in] offset       Offset into the \p mem to point to.
 * @param[in] num_elements Number of elements of \p mem referred to in the
 *                         by \p ref.
 *
 * @return error code.
 */
OSKAR_EXPORT
int oskar_mem_get_pointer(oskar_Mem* ref, const oskar_Mem* mem,
        const int offset, const int num_elements);

#ifdef __cplusplus
}
#endif

#endif // OSKAR_MEM_GET_POINTER_H_