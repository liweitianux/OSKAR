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

#include <oskar_global.h>

#include <apps/lib/oskar_OptionParser.h>
#include <utility/oskar_Mem.h>
#include <utility/oskar_mem_init.h>
#include <utility/oskar_mem_free.h>
#include <utility/oskar_get_error_string.h>
#include <station/oskar_StationModel.h>
#include <station/oskar_station_model_init.h>
#include <station/oskar_station_model_free.h>
#include <station/oskar_evaluate_array_pattern.h>
#include <station/oskar_evaluate_array_pattern_hierarchical.h>

#include <cuda_runtime_api.h>

#ifndef _WIN32
#   include <sys/time.h>
#endif /* _WIN32 */
#include <cstdlib>
#include <cstdio>

enum OpType { O2C, C2C, M2M, UNDEF };

int benchmark(int num_elements, int num_directions, OpType op_type,
        int loc, int precision, bool evaluate_2d, int niter, double& time_taken);


int main(int argc, char** argv)
{
    oskar_OptionParser opt("oskar_array_pattern_benchmark");
    opt.addRequired("No. array elements");
    opt.addRequired("No. directions");
    opt.addFlag("-o2c", "Single level beam pattern, phase only (real to complex DFT)");
    opt.addFlag("-c2c", "Beam pattern using complex inputs (complex to complex DFT)");
    opt.addFlag("-m2m", "Beam pattern using complex, polarised inputs (complex matrix to matrix DFT)");
    opt.addFlag("-g", "Evaluate the array pattern on the GPU");
    opt.addFlag("-c", "Evaluate the array pattern on the CPU");
    opt.addFlag("-sp", "Use single precision (default = double precision)");
    opt.addFlag("-2d", "Use a 2-dimensional phase term (default = 3d)");
    opt.addFlag("-n", "Number of iterations", 1, "1");
    opt.addFlag("-v", "Display verbose output.");

    if (!opt.check_options(argc, argv))
        return EXIT_FAILURE;

    int num_elements = atoi(opt.getArg(0));
    int num_directions = atoi(opt.getArg(1));
    OpType op_type = UNDEF;
    int op_type_count = 0;
    if (opt.isSet("-o2c"))
    {
        op_type = O2C;
        op_type_count++;
    }
    if (opt.isSet("-c2c"))
    {
        op_type = C2C;
        op_type_count++;
    }
    if (opt.isSet("-m2m"))
    {
        op_type = M2M;
        op_type_count++;
    }

    int loc;
    if (opt.isSet("-g"))
        loc = OSKAR_LOCATION_GPU;
    if (opt.isSet("-c"))
        loc = OSKAR_LOCATION_CPU;
    if (!(opt.isSet("-c") ^ opt.isSet("-g")))
    {
        opt.error("Please select one of -g or -c");
        return EXIT_FAILURE;
    }

    int precision = opt.isSet("-sp") ? OSKAR_SINGLE : OSKAR_DOUBLE;
    bool evaluate_2d = opt.isSet("-2d") ? true : false;
    int niter;
    opt.get("-n")->getInt(niter);

    if (op_type == UNDEF || op_type_count != 1) {
        opt.error("Please selected one of the following flags: -o2c, -c2c, -m2m");
        return EXIT_FAILURE;
    }

    if (opt.isSet("-v"))
    {
        printf("\n");
        printf("- Number of elements = %i\n", num_elements);
        printf("- Number of directions = %i\n", num_directions);
        printf("- Precision: %s\n", (precision == OSKAR_SINGLE) ? "single" : "double");
        printf("- %s\n", loc == OSKAR_LOCATION_CPU ? "CPU" : "GPU");
        printf("- %s\n", evaluate_2d ? "2D" : "3D");
        printf("- Evaluation type = ");
        if (op_type == O2C) printf("o2c\n");
        else if (op_type == C2C) printf("c2c\n");
        else if (op_type == M2M) printf("m2m\n");
        else printf("Error undefined!\n");
        printf("- Number of iterations = %i\n", niter);
        printf("\n");
    }

    double time_taken = 0.0;
    int status = benchmark(num_elements, num_directions, op_type, loc,
            precision, evaluate_2d, niter, time_taken);

    if (status) {
        fprintf(stderr, "ERROR: array pattern evaluation failed with code %i: "
                "%s\n", status, oskar_get_error_string(status));
        return EXIT_FAILURE;
    }
    if (opt.isSet("-v"))
    {
        printf("==> Total time taken = %f seconds.\n", time_taken);
        printf("==> Time taken per iteration = %f seconds.\n", time_taken/niter);
        printf("\n");
    }
    else {
        printf("%f\n", time_taken/niter);
    }

    return EXIT_SUCCESS;
}


int benchmark(int num_elements, int num_directions, OpType op_type,
        int loc, int precision, bool evaluate_2d, int niter, double& time_taken)
{
    int status = OSKAR_SUCCESS;

    // Create the CUDA events for timing.
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    oskar_StationModel station;
    oskar_station_model_init(&station, precision, loc, num_elements, &status);
    if (status) return status;
    station.array_is_3d = (evaluate_2d) ? OSKAR_FALSE : OSKAR_TRUE;
    station.coord_units = OSKAR_RADIANS;

    oskar_Mem x, y, z;
    oskar_mem_init(&x, precision, loc, num_directions, OSKAR_TRUE, &status);
    oskar_mem_init(&y, precision, loc, num_directions, OSKAR_TRUE, &status);
    oskar_mem_init(&z, precision, loc, num_directions, OSKAR_TRUE, &status);
    if (status) return status;

    oskar_Mem weights, beam, signal;

    if (op_type == O2C)
    {
        int type = precision | OSKAR_COMPLEX;
        oskar_mem_init(&beam, type, loc, num_directions, OSKAR_TRUE, &status);
        oskar_mem_init(&weights, type, loc, num_elements, OSKAR_TRUE, &status);
        if (status) return status;
        cudaDeviceSynchronize();

        float millisec = 0.0f;
        cudaEventRecord(start);
        for (int i = 0; i < niter; ++i)
        {
            oskar_evaluate_array_pattern(&beam, &station, num_directions, &x,
                    &y, &z, &weights, &status);
        }
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);
        cudaEventElapsedTime(&millisec, start, stop);
        time_taken = millisec / 1000.0;
    }
    else if (op_type == C2C || op_type == M2M)
    {
        int type = precision | OSKAR_COMPLEX;
        int num_signals = num_directions * num_elements;

        oskar_mem_init(&weights, type, loc, num_elements, OSKAR_TRUE, &status);
        if (op_type == C2C)
        {
            oskar_mem_init(&beam, type, loc, num_directions, OSKAR_TRUE, &status);
            oskar_mem_init(&signal, type, loc, num_signals, OSKAR_TRUE, &status);
        }
        else
        {
            type |= OSKAR_MATRIX;
            oskar_mem_init(&beam, type, loc, num_directions, OSKAR_TRUE, &status);
            oskar_mem_init(&signal, type, loc, num_signals, OSKAR_TRUE, &status);
        }
        if (status) return status;
        cudaDeviceSynchronize();

        float millisec = 0.0f;
        cudaEventRecord(start);
        for (int i = 0; i < niter; ++i)
        {
            oskar_evaluate_array_pattern_hierarchical(&beam, &station,
                    num_directions, &x, &y, &z, &signal, &weights, &status);
        }
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);
        cudaEventElapsedTime(&millisec, start, stop);
        time_taken = millisec / 1000.0;
    }

    // Destroy the timers.
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    // Free memory.
    oskar_station_model_free(&station, &status);
    oskar_mem_free(&x, &status);
    oskar_mem_free(&y, &status);
    oskar_mem_free(&z, &status);
    oskar_mem_free(&weights, &status);
    oskar_mem_free(&beam, &status);
    oskar_mem_free(&signal, &status);

    return status;
}
