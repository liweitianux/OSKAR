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
 *    and/or src materials provided with the distribution.
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

#include "oskar_global.h"
#include "sky/oskar_sky_model_append.h"
#include "utility/oskar_mem_append.h"
#include "utility/oskar_mem_realloc.h"

#ifdef __cplusplus
extern "C" {
#endif

int oskar_sky_model_append(oskar_SkyModel* dst, const oskar_SkyModel* src)
{
    int num_sources, error = 0;

    /* Check data dimensions. */
    num_sources = src->num_sources;
    if (num_sources > src->RA.num_elements  ||
            num_sources > src->Dec.num_elements ||
            num_sources > src->I.num_elements ||
            num_sources > src->Q.num_elements ||
            num_sources > src->U.num_elements ||
            num_sources > src->V.num_elements ||
            num_sources > src->reference_freq.num_elements ||
            num_sources > src->spectral_index.num_elements ||
            num_sources > src->FWHM_major.num_elements ||
            num_sources > src->FWHM_minor.num_elements ||
            num_sources > src->position_angle.num_elements)
    {
        return OSKAR_ERR_DIMENSION_MISMATCH;
    }

    /* Append to the sky model. */
    error = oskar_mem_append(&dst->RA, &src->RA);
    if (error) return error;
    error = oskar_mem_append(&dst->Dec, &src->Dec);
    if (error) return error;
    error = oskar_mem_append(&dst->I, &src->I);
    if (error) return error;
    error = oskar_mem_append(&dst->Q, &src->Q);
    if (error) return error;
    error = oskar_mem_append(&dst->U, &src->U);
    if (error) return error;
    error = oskar_mem_append(&dst->V, &src->V);
    if (error) return error;
    error = oskar_mem_append(&dst->reference_freq, &src->reference_freq);
    if (error) return error;
    error = oskar_mem_append(&dst->spectral_index, &src->spectral_index);
    if (error) return error;
    error = oskar_mem_append(&dst->FWHM_major, &src->FWHM_major);
    if (error) return error;
    error = oskar_mem_append(&dst->FWHM_minor, &src->FWHM_minor);
    if (error) return error;
    error = oskar_mem_append(&dst->position_angle, &src->position_angle);
    if (error) return error;

    /* Update the number of sources. */
    dst->num_sources += src->num_sources;

    /* Resize arrays to hold the direction cosines. */
    oskar_mem_realloc(&dst->rel_l, dst->num_sources, &error);
    oskar_mem_realloc(&dst->rel_m, dst->num_sources, &error);
    oskar_mem_realloc(&dst->rel_n, dst->num_sources, &error);

    /* Resize arrays to hold gaussian source parameters */
    oskar_mem_realloc(&dst->gaussian_a, dst->num_sources, &error);
    oskar_mem_realloc(&dst->gaussian_b, dst->num_sources, &error);
    oskar_mem_realloc(&dst->gaussian_c, dst->num_sources, &error);

    dst->use_extended = (src->use_extended || dst->use_extended);

    return error;
}

#ifdef __cplusplus
}
#endif
