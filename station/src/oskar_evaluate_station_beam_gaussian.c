/*
 * Copyright (c) 2013, The University of Oxford
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

#include "utility/oskar_vector_types.h"
#include "station/oskar_blank_below_horizon.h"
#include "station/oskar_evaluate_station_beam_gaussian.h"
#include "math/oskar_gaussian.h"
#include "math/oskar_gaussian_cuda.h"
#include "utility/oskar_mem_realloc.h"
#include "utility/oskar_mem_type_check.h"
#include "utility/oskar_cuda_check_error.h"

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifdef __cplusplus
extern "C" {
#endif

void oskar_evaluate_station_beam_gaussian(oskar_Mem* beam,
        int num_points, const oskar_Mem* l, const oskar_Mem* m,
        const oskar_Mem* horizon_mask, double fwhm_deg, int* status)
{
    int type, location;
    double fwhm_lm, std;

    /* Check all inputs. */
    if (!beam || !l || !m || !status)
    {
        oskar_set_invalid_argument(status);
        return;
    }

    /* Check if safe to proceed. */
    if (*status) return;

    /* Get type and check consistency. */
    type = oskar_mem_base_type(beam->type);
    if (type != l->type || type != m->type)
        *status = OSKAR_ERR_TYPE_MISMATCH;
    if (type != OSKAR_SINGLE && type != OSKAR_DOUBLE)
        *status = OSKAR_ERR_BAD_DATA_TYPE;
    if (!oskar_mem_is_complex(beam->type))
        *status = OSKAR_ERR_BAD_DATA_TYPE;

    if (fwhm_deg == 0.0)
    {
        *status = OSKAR_ERR_SETTINGS_TELESCOPE;
        return;
    }

    /* Get location and check consistency. */
    location = beam->location;
    if (location != l->location || location != m->location)
        *status = OSKAR_ERR_LOCATION_MISMATCH;

    /* Check that length of input arrays are consistent. */
    if (l->num_elements < num_points || m->num_elements < num_points)
        *status = OSKAR_ERR_DIMENSION_MISMATCH;

    /* Resize output array if needed. */
    if (beam->num_elements < num_points)
        oskar_mem_realloc(beam, num_points, status);

    /* Check if safe to proceed. */
    if (*status) return;

    /* Compute Gaussian standard deviation from FWHM. */
    fwhm_lm = sin(fwhm_deg * (M_PI / 180.0));
    std = fwhm_lm / (2.0 * sqrt(2.0 * log(2.0)));

    if (location == OSKAR_LOCATION_CPU)
    {
        if (type == OSKAR_DOUBLE)
        {
            if (oskar_mem_is_scalar(beam->type))
            {
                oskar_gaussian_d((double2*)beam->data, num_points,
                        (const double*)l->data, (const double*)m->data, std);
            }
            else
            {
                oskar_gaussian_md((double4c*)beam->data, num_points,
                        (const double*)l->data, (const double*)m->data, std);
            }
        }
        else /* type == OSKAR_SINGLE */
        {
            if (oskar_mem_is_scalar(beam->type))
            {
                oskar_gaussian_f((float2*)beam->data, num_points,
                        (const float*)l->data, (const float*)m->data,
                        (float)std);
            }
            else
            {
                oskar_gaussian_mf((float4c*)beam->data, num_points,
                        (const float*)l->data, (const float*)m->data,
                        (float)std);
            }
        }
    }
    /* GPU version. */
    else
    {
        if (type == OSKAR_DOUBLE)
        {
            if (oskar_mem_is_scalar(beam->type))
            {
                oskar_gaussian_cuda_d((double2*)beam->data, num_points,
                        (const double*)l->data, (const double*)m->data, std);
            }
            else
            {
                oskar_gaussian_cuda_md((double4c*)beam->data, num_points,
                        (const double*)l->data, (const double*)m->data, std);
            }
        }
        else /* type == OSKAR_SINGLE */
        {
            if (oskar_mem_is_scalar(beam->type))
            {
                oskar_gaussian_cuda_f((float2*)beam->data, num_points,
                        (const float*)l->data, (const float*)m->data,
                        (float)std);
            }
            else
            {
                oskar_gaussian_cuda_mf((float4c*)beam->data, num_points,
                        (const float*)l->data, (const float*)m->data,
                        (float)std);
            }
        }
        oskar_cuda_check_error(status);
    }

    /* Blank (zero) sources below the horizon. */
    oskar_blank_below_horizon(beam, horizon_mask, num_points, status);
}

#ifdef __cplusplus
}
#endif
