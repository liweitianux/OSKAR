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

#include "fits/test/Test_fits_to_sky_model.h"
#include "fits/oskar_fits_to_sky_model.h"
#include "fits/oskar_fits_image_write.h"
#include "imaging/oskar_Image.h"
#include "imaging/oskar_image_free.h"
#include "imaging/oskar_image_init.h"
#include "imaging/oskar_image_resize.h"
#include "sky/oskar_sky_model_save.h"
#include "utility/oskar_get_error_string.h"
#include "utility/oskar_Mem.h"

#include <fitsio.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#define FACTOR (2.0*sqrt(2.0*log(2.0)))

void Test_fits_to_sky_model::test_method()
{
    // Write a test image.
    int columns = 16; // width
    int rows = 16; // height
    int err = 0, status = 0;
    const char filename[] = "temp_test_sky_model.fits";
    fitsfile* fptr = NULL;
    int downsample_factor = 4;
    double bmaj = 10.0, bmin = 10.0; // arcsec
    double spectral_index = -0.7;

    // Create the image.
    oskar_Image image(OSKAR_DOUBLE, OSKAR_LOCATION_CPU);
    oskar_image_resize(&image, columns, rows, 1, 1, 1, &err);
    CPPUNIT_ASSERT_EQUAL(0, err);

    // Add image meta-data.
    image.centre_ra_deg = 10.0;
    image.centre_dec_deg = 80.0;
    image.fov_ra_deg = 0.1;
    image.fov_dec_deg = 0.1;
    image.freq_start_hz = 100e6;
    image.freq_inc_hz = 1e5;

    // Calculate "beam" area.
    double max = sin(image.fov_ra_deg * M_PI / 360.0); /* Divide by 2. */
    double inc = max / (0.5 * image.width);
    double cdelt1 = -asin(inc) * 180.0 / M_PI;
    double beam_area = 2.0 * M_PI * (bmaj * bmin)
                / (FACTOR * FACTOR * cdelt1 * cdelt1 * 3600.0 * 3600.0);

    // Define test input data.
    double* d = (double*) image.data;
    for (int r = 0, i = 0; r < rows; ++r)
    {
        for (int c = 0; c < columns; ++c, ++i)
        {
            if ((c % downsample_factor == 1) &&
                    (r % downsample_factor == 1))
                d[i] = 1.0;
            else
                d[i] = 0.0001;
        }
    }

    // Write the data.
    oskar_fits_image_write(&image, NULL, filename);

    // Re-open the FITS file.
    fits_open_file(&fptr, filename, READWRITE, &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Add a fake beam size.
    fits_write_key_dbl(fptr, "BMAJ", bmaj / 3600.0, 10,
            "Beam major axis (deg)", &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));
    fits_write_key_dbl(fptr, "BMIN", bmin / 3600.0, 10,
            "Beam minor axis (deg)", &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Close the FITS file.
    fits_close_file(fptr, &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Load the sky model (no downsampling, no noise floor).
    {
        oskar_SkyModel sky_temp;
        err = oskar_fits_to_sky_model(0, filename, &sky_temp, spectral_index,
                0.0, 0.0, 0);
        if (err) CPPUNIT_FAIL(oskar_get_error_string(err));
        CPPUNIT_ASSERT_EQUAL(rows * columns, sky_temp.num_sources);

        for (int r = 0, i = 0; r < rows; ++r)
        {
            for (int c = 0; c < columns; ++c, ++i)
            {
                if ((c % downsample_factor == 1) &&
                        (r % downsample_factor == 1))
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 / beam_area,
                            ((double*)sky_temp.I.data)[i], 1e-5);
                else
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0001 / beam_area,
                            ((double*)sky_temp.I.data)[i], 1e-5);
            }
        }

        oskar_sky_model_save("test_grid.osm", &sky_temp, &err);
    }

    // Load the sky model (with downsampling, with noise floor).
    {
        oskar_SkyModel sky_temp;
        err = oskar_fits_to_sky_model(0, filename, &sky_temp, spectral_index,
                0.0, 0.01, downsample_factor);
        if (err) CPPUNIT_FAIL(oskar_get_error_string(err));
        CPPUNIT_ASSERT_EQUAL((columns / downsample_factor) *
                (rows / downsample_factor), sky_temp.num_sources);

        for (int i = 0; i < sky_temp.num_sources; ++i)
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 / beam_area,
                    ((double*)sky_temp.I.data)[i], 1e-5);
        }
    }

    // Define test input data.
    for (int r = 0, i = 0; r < rows; ++r)
    {
        for (int c = 0; c < columns; ++c, ++i)
        {
            if ((c % downsample_factor == 1) &&
                    (r % downsample_factor == 1))
                d[i] = 10.0;
            else
                d[i] = 1;
        }
    }

    // Write the data.
    oskar_fits_image_write(&image, NULL, filename);

    // Re-open the FITS file.
    fits_open_file(&fptr, filename, READWRITE, &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Add a fake beam size.
    fits_write_key_dbl(fptr, "BMAJ", bmaj / 3600.0, 10,
            "Beam major axis (deg)", &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));
    fits_write_key_dbl(fptr, "BMIN", bmin / 3600.0, 10,
            "Beam minor axis (deg)", &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Close the FITS file.
    fits_close_file(fptr, &status);
    if (status) CPPUNIT_FAIL(oskar_get_error_string(OSKAR_ERR_FITS_IO));

    // Load the sky model (with downsampling, no noise floor).
    {
        oskar_SkyModel sky_temp;
        err = oskar_fits_to_sky_model(0, filename, &sky_temp, spectral_index,
                0.0, 0.0, downsample_factor);
        if (err) CPPUNIT_FAIL(oskar_get_error_string(err));
        CPPUNIT_ASSERT_EQUAL(((columns + downsample_factor - 1) / downsample_factor)
                * ((rows + downsample_factor - 1) / downsample_factor),
                sky_temp.num_sources);

        double expected = ((downsample_factor * downsample_factor - 1) + 10.0)
                / beam_area;
        for (int i = 0; i < sky_temp.num_sources; ++i)
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected,
                    ((double*)sky_temp.I.data)[i], 1e-5);
        }
    }

    // Test real astronomical FITS images.
    {
        oskar_SkyModel sky_Cyg_A_4, sky_Cyg_A_3, sky_Cas_A_2;

        // Read Cyg A model.
        oskar_fits_to_sky_model(0, "Cyg_A-P.model.FITS",
                &sky_Cyg_A_4, spectral_index, 0.02, 0.0, 4);
        oskar_sky_model_save("temp_test_Cyg_A_model_4.osm", &sky_Cyg_A_4, &err);

        // Read Cyg A model.
        oskar_fits_to_sky_model(0, "Cyg_A-P.model.FITS",
                &sky_Cyg_A_3, spectral_index, 0.02, 0.0, 3);
        oskar_sky_model_save("temp_test_Cyg_A_model_3.osm", &sky_Cyg_A_3, &err);

        // Read Cas A model.
        oskar_fits_to_sky_model(0, "Cas_A-P.models.FITS",
                &sky_Cas_A_2, spectral_index, 0.02, 0.0, 2);
        oskar_sky_model_save("temp_test_Cas_A_model_2.osm", &sky_Cas_A_2, &err);
    }
    {
        oskar_SkyModel sky_Cyg_A, sky_Cas_A;

        // Read Cyg A model.
        oskar_fits_to_sky_model(0, "Cyg_A-P.model.FITS",
                &sky_Cyg_A, spectral_index, 0.02, 0.0, 1);
        oskar_sky_model_save("temp_test_Cyg_A_model_1.osm", &sky_Cyg_A, &err);

        // Read Cas A model.
        oskar_fits_to_sky_model(0, "Cas_A-P.models.FITS",
                &sky_Cas_A, spectral_index, 0.02, 0.0, 1);
        oskar_sky_model_save("temp_test_Cas_A_model_1.osm", &sky_Cas_A, &err);
    }

    // Free memory.
    oskar_image_free(&image, &err);
}
