/*
 * Copyright (c) 2012-2014, The University of Oxford
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

#ifndef OSKAR_MEM_H_
#define OSKAR_MEM_H_

/**
 * @file oskar_mem.h
 */

#include <oskar_global.h>

/* Public interface. */

#ifdef __cplusplus
extern "C" {
#endif

struct oskar_Mem;
#ifndef OSKAR_MEM_TYPEDEF_
#define OSKAR_MEM_TYPEDEF_
typedef struct oskar_Mem oskar_Mem;
#endif /* OSKAR_MEM_TYPEDEF_ */

/* Define an enumerator for the type.
 *
 * IMPORTANT:
 * 1. All these must be small enough to fit into one byte (8 bits) only.
 * 2. To maintain binary data compatibility, do not modify any numbers
 *    that appear in the list below!
 */
enum
{
    /* Byte (char): bit 0 set. */
    OSKAR_CHAR                   = 0x01,

    /* Integer (int): bit 1 set. */
    OSKAR_INT                    = 0x02,

    /* Scalar single (float): bit 2 set. */
    OSKAR_SINGLE                 = 0x04,

    /* Scalar double (double): bit 3 set. */
    OSKAR_DOUBLE                 = 0x08,

    /* Complex flag: bit 5 set. */
    OSKAR_COMPLEX                = 0x20,

    /* Matrix flag: bit 6 set. */
    OSKAR_MATRIX                 = 0x40,

    /* Scalar complex single (float2). */
    OSKAR_SINGLE_COMPLEX         = OSKAR_SINGLE | OSKAR_COMPLEX,

    /* Scalar complex double (double2). */
    OSKAR_DOUBLE_COMPLEX         = OSKAR_DOUBLE | OSKAR_COMPLEX,

    /* Matrix complex float (float4c). */
    OSKAR_SINGLE_COMPLEX_MATRIX  = OSKAR_SINGLE | OSKAR_COMPLEX | OSKAR_MATRIX,

    /* Matrix complex double (double4c). */
    OSKAR_DOUBLE_COMPLEX_MATRIX  = OSKAR_DOUBLE | OSKAR_COMPLEX | OSKAR_MATRIX
};

enum
{
    OSKAR_CPU = 0,
    OSKAR_GPU = 1
};

/**
 * @brief
 * Returns the base type (precision) of an OSKAR memory element enumerator.
 *
 * @details
 * Returns the base type of an OSKAR memory data element enumerator
 * (OSKAR_SINGLE, OSKAR_DOUBLE, OSKAR_INT, or OSKAR_CHAR), ignoring complex
 * or matrix types.
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return The base type of the memory.
 */
OSKAR_INLINE int oskar_mem_type_precision(const int mem_type)
{
    return (mem_type & 0x0F);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is double precision.
 *
 * @details
 * Returns 1 (true) if the memory element is double precision, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if double, 0 if single.
 */
OSKAR_INLINE int oskar_mem_type_is_double(const int mem_type)
{
    return ((mem_type & 0x0F) == OSKAR_DOUBLE);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is single precision.
 *
 * @details
 * Returns 1 (true) if the memory element is single precision, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if single, 0 if double.
 */
OSKAR_INLINE int oskar_mem_type_is_single(const int mem_type)
{
    return ((mem_type & 0x0F) == OSKAR_SINGLE);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is complex.
 *
 * @details
 * Returns 1 (true) if the memory element is complex, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if complex, 0 if real.
 */
OSKAR_INLINE int oskar_mem_type_is_complex(const int mem_type)
{
    return ((mem_type & OSKAR_COMPLEX) == OSKAR_COMPLEX);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is real.
 *
 * @details
 * Returns 1 (true) if the memory element is real, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if real, 0 if complex.
 */
OSKAR_INLINE int oskar_mem_type_is_real(const int mem_type)
{
    return ((mem_type & OSKAR_COMPLEX) == 0);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is matrix.
 *
 * @details
 * Returns 1 (true) if the memory element is matrix, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if matrix, 0 if scalar.
 */
OSKAR_INLINE int oskar_mem_type_is_matrix(const int mem_type)
{
    return ((mem_type & OSKAR_MATRIX) == OSKAR_MATRIX);
}

/**
 * @brief
 * Checks if the OSKAR memory pointer data element is scalar.
 *
 * @details
 * Returns 1 (true) if the memory element is scalar, else 0 (false).
 *
 * @param[in] mem_type Type of oskar_Mem structure.
 *
 * @return 1 if scalar, 0 if matrix.
 */
OSKAR_INLINE int oskar_mem_type_is_scalar(const int mem_type)
{
    return ((mem_type & OSKAR_MATRIX) == 0);
}

#ifdef __cplusplus
}
#endif

#include <oskar_mem_accessors.h>
#include <oskar_mem_add.h>
#include <oskar_mem_append_raw.h>
#include <oskar_mem_clear_contents.h>
#include <oskar_mem_copy.h>
#include <oskar_mem_copy_contents.h>
#include <oskar_mem_convert_precision.h>
#include <oskar_mem_create.h>
#include <oskar_mem_create_alias.h>
#include <oskar_mem_create_alias_from_raw.h>
#include <oskar_mem_create_copy.h>
#include <oskar_mem_data_type_string.h>
#include <oskar_mem_different.h>
#include <oskar_mem_element_multiply.h>
#include <oskar_mem_element_size.h>
#include <oskar_mem_evaluate_relative_error.h>
#include <oskar_mem_free.h>
#include <oskar_mem_get_element.h>
#include <oskar_mem_load_ascii.h>
#include <oskar_mem_random_fill.h>
#include <oskar_mem_read_binary_raw.h>
#include <oskar_mem_realloc.h>
#include <oskar_mem_save_ascii.h>
#include <oskar_mem_scale_real.h>
#include <oskar_mem_set_alias.h>
#include <oskar_mem_set_element.h>
#include <oskar_mem_set_value_real.h>
#include <oskar_mem_stats.h>

#endif /* OSKAR_MEM_H_ */
