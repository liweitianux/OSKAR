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

#ifndef OSKAR_BINARY_TAG_INDEX_QUERY_H_
#define OSKAR_BINARY_TAG_INDEX_QUERY_H_

/**
 * @file oskar_binary_tag_index_query.h
 */

#include "oskar_global.h"

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return the block size, data size and offset associated with a tag.
 *
 * @details
 * This function returns the block size, data size and offset associated with
 * a given tag.
 *
 * Tags can be specified either as two bytes or as two strings, if the tag
 * is an extended tag. If the tag is not extended, pass NULL for the name
 * arguments.
 *
 * It is an error to specify both a group/tag ID and group/tag names:
 * when using names, set the IDs to zero.
 *
 * @param[in] index        Index structure pointer.
 * @param[in] data_type    Type of the memory (as in oskar_Mem).
 * @param[in] id_group     Tag group identifier.
 * @param[in] id_tag       Tag identifier.
 * @param[in] name_group   Tag group name (NULL if not an extended tag).
 * @param[in] name_tag     Tag name (NULL if not an extended tag).
 * @param[in] user_index   User-defined index.
 * @param[out] block_size  The total size of the block, in bytes.
 * @param[out] data_size   The size of the data, in bytes.
 * @param[out] data_offset The data offset from the start of the file, in bytes.
 */
OSKAR_EXPORT
int oskar_binary_tag_index_query(const oskar_BinaryTagIndex* index,
        unsigned char data_type, unsigned char id_group, unsigned char id_tag,
        const char* name_group, const char* name_tag, int user_index,
        size_t* block_size, size_t* data_size, long* data_offset);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_BINARY_TAG_INDEX_QUERY_H_ */
