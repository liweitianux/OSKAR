/*
 * Copyright (c) 2015, The University of Oxford
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

#include <private_vis_block.h>
#include <oskar_vis_block.h>

#ifdef __cplusplus
extern "C" {
#endif

void oskar_vis_block_copy(oskar_VisBlock* dst, const oskar_VisBlock* src,
        int* status)
{
    /* Check all inputs. */
    if (!dst || !src || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Copy the meta-data. */
    dst->dim_size[0] = src->dim_size[0];
    dst->dim_size[1] = src->dim_size[1];
    dst->dim_size[2] = src->dim_size[2];
    dst->dim_size[3] = src->dim_size[3];
    dst->freq_range_hz[0] = src->freq_range_hz[0];
    dst->freq_range_hz[1] = src->freq_range_hz[1];
    dst->time_range_mjd_utc_sec[0] = src->time_range_mjd_utc_sec[0];
    dst->time_range_mjd_utc_sec[1] = src->time_range_mjd_utc_sec[1];

    /* Copy the memory. */
    oskar_mem_copy(dst->baseline_uu_metres, src->baseline_uu_metres, status);
    oskar_mem_copy(dst->baseline_vv_metres, src->baseline_vv_metres, status);
    oskar_mem_copy(dst->baseline_ww_metres, src->baseline_ww_metres, status);
    oskar_mem_copy(dst->baseline_num_channel_averages,
            src->baseline_num_channel_averages, status);
    oskar_mem_copy(dst->baseline_num_time_averages,
            src->baseline_num_time_averages, status);
    oskar_mem_copy(dst->amplitude, src->amplitude, status);
    oskar_mem_copy(dst->a1, src->a1, status);
    oskar_mem_copy(dst->a2, src->a2, status);
}

#ifdef __cplusplus
}
#endif