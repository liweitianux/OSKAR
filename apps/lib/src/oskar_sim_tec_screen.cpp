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

#include "apps/lib/oskar_sim_tec_screen.h"

#include "apps/lib/oskar_settings_load.h"
#include "apps/lib/oskar_set_up_telescope.h"

#include <sky/oskar_evaluate_TEC_TID.h>
#include <sky/oskar_SettingsIonosphere.h>
#include <sky/oskar_mjd_to_gast_fast.h>
#include <sky/oskar_ra_dec_to_hor_lmn.h>
#include <utility/oskar_Settings.h>
#include <utility/oskar_Mem.h>
#include <utility/oskar_mem_init.h>
#include <utility/oskar_mem_free.h>
#include <utility/oskar_mem_set_value_real.h>
#include <utility/oskar_mem_get_pointer.h>
#include <utility/oskar_log_settings.h>
#include <interferometry/oskar_TelescopeModel.h>
#include <interferometry/oskar_offset_geocentric_cartesian_to_geocentric_cartesian.h>
#include <imaging/oskar_evaluate_image_lm_grid.h>
#include <imaging/oskar_image_free.h>
#include <imaging/oskar_image_init.h>
#include <imaging/oskar_image_resize.h>
#include <imaging/oskar_image_write.h>
#include <fits/oskar_fits_image_write.h>
#include <math/oskar_sph_from_lm.h>
#include <station/oskar_StationModel.h>
#include <station/oskar_evaluate_pierce_points.h>

#include <cmath>
#include <cstdlib>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void evalaute_station_beam_pp(double* pp_lon0, double* pp_lat0,
        int stationID, oskar_Settings* settings,
        oskar_TelescopeModel* telescope, int* status);

extern "C"
int oskar_sim_tec_screen(const char* settings_file, oskar_Log* log)
{
    int status = OSKAR_SUCCESS;

    // Settings.
    oskar_Settings settings;
    oskar_settings_load(&settings, log, settings_file);
    oskar_SettingsIonosphere* MIM = &settings.ionosphere;
    log->keep_file = settings.sim.keep_log_file;

    oskar_TelescopeModel telescope;
    oskar_set_up_telescope(&telescope, log, &settings, &status);

    oskar_log_settings_telescope(log, &settings);
    oskar_log_settings_observation(log, &settings);
    oskar_log_settings_ionosphere(log, &settings);

    if (!settings.ionosphere.TECImage.fits_file &&
            !settings.ionosphere.TECImage.img_file)
    {
        return OSKAR_ERR_SETTINGS_IONOSPHERE;
    }

    int im_size = settings.ionosphere.TECImage.size;
    int num_pixels = im_size * im_size;

    int type = settings.sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;

    int loc = OSKAR_LOCATION_CPU;

    int owner = OSKAR_TRUE;
    double fov = settings.ionosphere.TECImage.fov_rad;

    // Evaluate the p.p. coordinates of the beam phase centre.
    double pp_lon0, pp_lat0;
    int st_idx = settings.ionosphere.TECImage.stationID;
    if (settings.ionosphere.TECImage.beam_centred)
    {
        evalaute_station_beam_pp(&pp_lon0, &pp_lat0, st_idx, &settings,
                &telescope, &status);
    }
    else
    {
        pp_lon0 = telescope.station[st_idx].beam_longitude_rad;
        pp_lat0 = telescope.station[st_idx].beam_latitude_rad;
    }

    int num_times = settings.obs.num_time_steps;
    double t0 = settings.obs.start_mjd_utc;
    double tinc = settings.obs.dt_dump_days;

    // Work out the lon, lat grid used for the tec values.
    oskar_Mem grid_l, grid_m, pp_lon, pp_lat;
    oskar_mem_init(&grid_l, type, loc, num_pixels, owner, &status);
    oskar_mem_init(&grid_m, type, loc, num_pixels, owner, &status);
    oskar_mem_init(&pp_lon, type, loc, num_pixels, owner, &status);
    oskar_mem_init(&pp_lat, type, loc, num_pixels, owner, &status);

    if (type == OSKAR_DOUBLE)
    {
        oskar_evaluate_image_lm_grid_d(im_size, im_size, fov, fov,
                (double*)(grid_l.data), (double*)(grid_m.data));
        oskar_sph_from_lm_d(num_pixels, pp_lon0, pp_lat0,
                (double*)(grid_l.data), (double*)(grid_m.data),
                (double*)(pp_lon.data), (double*)(pp_lat.data));
    }
    else
    {
        oskar_evaluate_image_lm_grid_f(im_size, im_size, fov, fov,
                (float*)(grid_l.data), (float*)(grid_m.data));
        oskar_sph_from_lm_f(num_pixels, pp_lon0, pp_lat0,
                (float*)(grid_l.data), (float*)(grid_m.data),
                (float*)(pp_lon.data), (float*)(pp_lat.data));
    }
    // Relative path in direction of pp (1.0 here as we are not using
    // any stations)
    oskar_Mem pp_rel_path;
    oskar_mem_init(&pp_rel_path, type, loc, num_pixels, owner, &status);
    oskar_mem_set_value_real(&pp_rel_path, 1.0, &status);

    // Initialise return values
    oskar_Image tec_image;
    oskar_image_init(&tec_image, type, loc, &status);
    oskar_image_resize(&tec_image, im_size, im_size, 1, num_times,
            1, &status);
    tec_image.image_type = OSKAR_IMAGE_TYPE_BEAM_SCALAR;
    tec_image.centre_ra_deg = pp_lon0;
    tec_image.centre_dec_deg = pp_lat0;
    tec_image.fov_ra_deg = fov * (180.0/M_PI);
    tec_image.fov_dec_deg = fov * (180.0/M_PI);
    tec_image.freq_start_hz = 0.0;
    tec_image.freq_inc_hz = 0.0;
    tec_image.time_inc_sec = tinc * 86400.;
    tec_image.time_start_mjd_utc = t0;

    oskar_Mem tec_screen;
    oskar_mem_init(&tec_screen, type, loc, num_pixels, !owner, &status);

    for (int i = 0; i < num_times; ++i)
    {
        double gast = t0 + tinc * (double)i;
        int offset = num_pixels * i;
        oskar_mem_get_pointer(&tec_screen, &(tec_image.data), offset,
                num_pixels, &status);
        oskar_evaluate_TEC_TID(&tec_screen, num_pixels,
                &pp_lon, &pp_lat, &pp_rel_path, MIM->TEC0,
                &(MIM->TID[0]), gast);
    }

    if (!status)
    {
        // Write FITS image.
        const char* fits_file = settings.ionosphere.TECImage.fits_file;
        if (fits_file)
            status = oskar_fits_image_write(&tec_image, log, fits_file);
        const char* img_file = settings.ionosphere.TECImage.img_file;
        if (img_file)
            oskar_image_write(&tec_image, log, img_file, 0, &status);
    }

    oskar_mem_free(&grid_l, &status);
    oskar_mem_free(&grid_m, &status);
    oskar_mem_free(&pp_lon, &status);
    oskar_mem_free(&pp_lat, &status);
    oskar_mem_free(&pp_rel_path, &status);
    oskar_image_free(&tec_image, &status);

    return status;
}





static void evalaute_station_beam_pp(double* pp_lon0, double* pp_lat0,
        int stationID, oskar_Settings* settings, oskar_TelescopeModel* telescope,
        int* status)
{
    int type = settings->sim.double_precision ? OSKAR_DOUBLE : OSKAR_SINGLE;

    int loc = OSKAR_LOCATION_CPU;
    int owner = OSKAR_TRUE;

    oskar_StationModel* station = &telescope->station[stationID];

    // oskar_Mem holding beam p.p. horizontal coordinates.
    oskar_Mem hor_x, hor_y, hor_z;
    oskar_mem_init(&hor_x, type, loc, 1, owner, status);
    oskar_mem_init(&hor_y, type, loc, 1, owner, status);
    oskar_mem_init(&hor_z, type, loc, 1, owner, status);

    /* Offset geocentric cartesian station position */
    double st_x, st_y, st_z;

    // ECEF coordinates of the station for which the beam p.p. is being evaluated.
    double st_x_ecef, st_y_ecef, st_z_ecef;

    double st_lon = station->longitude_rad;
    double st_lat = station->latitude_rad;
    double st_alt = station->altitude_m;

    // Time at which beam p.p. is evaluated.
    int t = 0;
    double obs_start_mjd_utc = settings->obs.start_mjd_utc;
    double dt_dump = settings->obs.dt_dump_days;
    double t_dump = obs_start_mjd_utc + t * dt_dump; // MJD UTC
    double gast = oskar_mjd_to_gast_fast(t_dump + dt_dump / 2.0);
    double last = gast + st_lon;

    if (type == OSKAR_DOUBLE)
    {
        st_x = ((double*)telescope->station_x.data)[stationID];
        st_y = ((double*)telescope->station_y.data)[stationID];
        st_z = ((double*)telescope->station_z.data)[stationID];

        oskar_offset_geocentric_cartesian_to_geocentric_cartesian(1,
                &st_x, &st_y, &st_z, st_lon, st_lat, st_alt, &st_x_ecef,
                &st_y_ecef, &st_z_ecef);

        double beam_ra = station->beam_longitude_rad;
        double beam_dec = station->beam_latitude_rad;

        // Obtain horizontal coordinates of beam p.p.
        oskar_ra_dec_to_hor_lmn_d(1, &beam_ra, &beam_dec, last, st_lat,
                (double*)hor_x.data, (double*)hor_y.data, (double*)hor_z.data);
    }
    else // (type == OSKAR_SINGLE)
    {
        st_x = (double)((float*)telescope->station_x.data)[stationID];
        st_y = (double)((float*)telescope->station_y.data)[stationID];
        st_z = (double)((float*)telescope->station_z.data)[stationID];

        oskar_offset_geocentric_cartesian_to_geocentric_cartesian(1,
                &st_x, &st_y, &st_z, st_lon, st_lat, st_alt, &st_x_ecef,
                &st_y_ecef, &st_z_ecef);

        float beam_ra = (float)station->beam_longitude_rad;
        float beam_dec = (float)station->beam_latitude_rad;

        // Obtain horizontal coordinates of beam p.p.
        oskar_ra_dec_to_hor_lmn_f(1, &beam_ra, &beam_dec, last, st_lat,
                (float*)hor_x.data, (float*)hor_y.data, (float*)hor_z.data);
    }

    // oskar_Mem functions holding the pp for the beam centre.
    oskar_Mem m_pp_lon0, m_pp_lat0, m_pp_rel_path;
    oskar_mem_init(&m_pp_lon0, type, loc, 1, owner, status);
    oskar_mem_init(&m_pp_lat0, type, loc, 1, owner, status);
    oskar_mem_init(&m_pp_rel_path, type, loc, 1, owner, status);

    // Pierce point of the observation phase centre - i.e. beam direction
    oskar_evaluate_pierce_points(
            &m_pp_lon0, &m_pp_lat0, &m_pp_rel_path,
            st_lon, st_lat, st_alt,
            st_x_ecef, st_y_ecef, st_z_ecef,
            settings->ionosphere.TID[0].height_km * 1000.,
            1,
            &hor_x, &hor_y, &hor_z,
            status);

    if (type == OSKAR_DOUBLE)
    {
        *pp_lon0 = ((double*)m_pp_lon0.data)[0];
        *pp_lat0 = ((double*)m_pp_lat0.data)[0];
    }
    else
    {
        *pp_lon0 = ((float*)m_pp_lon0.data)[0];
        *pp_lat0 = ((float*)m_pp_lat0.data)[0];
    }

//    printf("pp lon = %f, lat = %f\n", *pp_lon0*(180./M_PI),
//            *pp_lat0*(180./M_PI));
}
