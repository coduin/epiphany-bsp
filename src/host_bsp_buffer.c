/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/

#include "host_bsp_private.h"

#include <stdio.h>
#include <string.h>


extern bsp_state_t state;
#define MINIMUM_CHUNK_SIZE (4 * sizeof(int))

void* bsp_stream_create(int stream_size, int token_size,
                         const void* initial_data) {
    if (token_size < MINIMUM_CHUNK_SIZE) {
        printf("ERROR: minimum token size is %i bytes\n", MINIMUM_CHUNK_SIZE);
        return 0;
    }
    if (state.combuf.nstreams == MAX_N_STREAMS) {
        printf("ERROR: Reached limit of %d streams.\n", MAX_N_STREAMS);
        return 0;
    }

    // Amount of tokens, rounded up
    int ntokens = (stream_size + token_size - 1) / token_size;

    int nbytes_including_headers =
        stream_size + ntokens * 2 * sizeof(int) +
        2 * sizeof(int); // the +2*sizeof(int) is the terminating header
                         // headers consist of 2 ints: prev size and next size

    // 1) malloc in extmem
    void* extmem_buffer = ebsp_ext_malloc(nbytes_including_headers);
    if (extmem_buffer == 0) {
        printf("ERROR: not enough memory in extmem for ebsp_stream_create\n");
        return 0;
    }

    // 2) copy the data to extmem, inserting headers
    unsigned dst_cursor = (unsigned)extmem_buffer;
    unsigned src_cursor = (unsigned)initial_data;

    if (initial_data) {
        int current_chunksize = token_size;
        int last_chunksize = 0;
        for (int nbytes_left = stream_size; nbytes_left > 0;
             nbytes_left -= token_size) {
            if (nbytes_left < token_size)
                current_chunksize = nbytes_left;

            (*(int*)dst_cursor) = last_chunksize; // write prev header
            dst_cursor += sizeof(int);
            (*(int*)dst_cursor) = current_chunksize; // write next header
            dst_cursor += sizeof(int);

            memcpy((void*)dst_cursor, (void*)src_cursor, current_chunksize);

            dst_cursor += current_chunksize;
            src_cursor += current_chunksize;

            last_chunksize = current_chunksize;
        }
        // Write a terminating header
        (*(int*)dst_cursor) = current_chunksize; // write terminating header (prev)
        dst_cursor += sizeof(int);
        (*(int*)dst_cursor) = 0; // write terminating header (next)
        dst_cursor += sizeof(int);
    } else {
        // Write a terminating header, or upstreams will crash
        (*(int*)dst_cursor) = 0; // prevsize
        dst_cursor += sizeof(int);
        (*(int*)dst_cursor) = 0; // nextsize
        dst_cursor += sizeof(int);
    }

    // 3) add stream to combuf
    ebsp_stream_descriptor x;

    x.extmem_addr = _arm_to_e_pointer(extmem_buffer);
    x.cursor = x.extmem_addr;
    x.nbytes = nbytes_including_headers;
    x.max_chunksize = token_size;
    x.pid = -1;
    memset(&x.e_dma_desc, 0, sizeof(ebsp_dma_handle));
    x.current_buffer = NULL;
    x.next_buffer = NULL;

    state.shared_streams[state.combuf.nstreams] = x;
    state.combuf.nstreams++;

    return extmem_buffer;
}
