/*
 * Copyright (c) 2012, The University of Oxford
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

#include "utility/oskar_mem_clear_contents.h"
#include "utility/oskar_mem_element_size.h"
#include "utility/oskar_Mem.h"

#include <cuda_runtime_api.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_mem_clear_contents(oskar_Mem* mem, int* status)
{
    size_t size;

    /* Check all inputs. */
    if (!mem || !status)
    {
        if (status) *status = OSKAR_ERR_INVALID_ARGUMENT;
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Compute the size. */
    size = mem->num_elements * oskar_mem_element_size(mem->type);

    /* Clear the memory. */
    if (mem->location == OSKAR_LOCATION_CPU)
    {
        memset(mem->data, 0, size);
    }
    else if (mem->location == OSKAR_LOCATION_GPU)
    {
        cudaMemset(mem->data, 0, size);
        *status = cudaPeekAtLastError();
    }
    else
    {
        *status = OSKAR_ERR_BAD_LOCATION;
    }
}

#ifdef __cplusplus
}
#endif
