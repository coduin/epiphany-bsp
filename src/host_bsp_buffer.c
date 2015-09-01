/*
File: host_bsp_buffer.c

This file is part of the Epiphany BSP library.

Copyright (C) 2014 Buurlage Wits
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


void ebsp_send_buffered(void* src, int dst_core_id, int nbytes, int max_chunksize)
{
    int nchunks = (nbytes + max_chunksize - 1)/max_chunksize; // nbytes/chunksize rounded up

    int nbytes_including_headers = nbytes + nchunks*2*sizeof(int) + 2*sizeof(int); // the +2*sizeof(int) is the terminating header
                                        // headers consist of 2 ints: prev size and next size

    // 1) malloc in extmem
    void* extmem_in_buffer = ebsp_ext_malloc(nbytes_including_headers);
    if (extmem_in_buffer == 0)
    {
        printf("ERROR: not enough memory in extmem for ebsp_send_buffered_raw\n");
        return;
    }

    // 2) copy the data to extmem, inserting headers
    unsigned dst_cursor = (unsigned)extmem_in_buffer;
    unsigned src_cursor = (unsigned)src;
    
    int current_chunksize = max_chunksize;
    int last_chunksize = 0;
    for (int nbytes_left = nbytes; nbytes_left > 0; nbytes_left -= max_chunksize)
    {
        if (nbytes_left < max_chunksize)
            current_chunksize = nbytes_left;

        (*(int*)dst_cursor) = last_chunksize; // write prev header
        dst_cursor += sizeof(int);
        (*(int*)dst_cursor) = current_chunksize; // write next header
        dst_cursor += sizeof(int);
        
        memcpy((void*) dst_cursor, (void*) src_cursor, current_chunksize);
        
        dst_cursor += current_chunksize;
        src_cursor += current_chunksize;

        last_chunksize = current_chunksize;
    }

    (*(int*)dst_cursor) = current_chunksize; // write terminating header (prev)
    dst_cursor += sizeof(int);
    (*(int*)dst_cursor) = 0; // write terminating header (next)
    dst_cursor += sizeof(int);

    // 3) add stream to state
    _ebsp_add_stream(dst_core_id, extmem_in_buffer, nbytes_including_headers, max_chunksize);
}

void ebsp_send_buffered_raw(void* src, int dst_core_id, int nbytes, int max_chunksize)
{
    // 1) malloc in extmem
    void* extmem_in_buffer = ebsp_ext_malloc(nbytes);
    if (extmem_in_buffer == 0)
    {
        printf("ERROR: not enough memory in extmem for ebsp_send_buffered_raw\n");
        return;
    }
    // 2) copy the data there directly
    memcpy(extmem_in_buffer, src, nbytes);

    // 3) add stream to state
    _ebsp_add_stream(dst_core_id, (void*)extmem_in_buffer, nbytes, max_chunksize);
}


// add ebsp_stream_descriptor to state.buffered_in_streams, update state.n_in_streams
void _ebsp_add_stream(int dst_core_id, void* extmem_in_buffer, int nbytes, int max_chunksize)
{
    if (state.combuf.n_in_streams[dst_core_id] == MAX_N_IN_STREAMS)
    {
        printf("ERROR: state.combuf.n_in_streams >= MAX_N_IN_STREAMS\n");
        return;
    }

    ebsp_in_stream_descriptor x;

    x.extmem_in_addr    = _arm_to_e_pointer(extmem_in_buffer);
    x.in_cursor         = x.extmem_in_addr;
    x.nbytes            = nbytes;
    x.max_chunksize     = max_chunksize;
    memset(&x.e_dma_desc, 0, sizeof(e_dma_desc_host_t));
    x.current_in_buffer = NULL;
    x.next_in_buffer    = NULL;

    state.buffered_in_streams[dst_core_id][state.combuf.n_in_streams[dst_core_id]] = x;
    state.combuf.n_in_streams[dst_core_id]++;
}


void ebsp_get_buffered(int nbytes)
{
    // 1) malloc in extmem
    void* extmem_out_buffer = ebsp_ext_malloc(nbytes);
    if (extmem_in_buffer == 0)
    {
        printf("ERROR: not enough memory in extmem for ebsp_get_buffered\n");
        return;
    }

    // 2) add stream to state
    _ebsp_add_stream(dst_core_id, extmem_in_buffer, nbytes_including_headers, max_chunksize);
}



/*
void ebsp_get_buffered(int dst_core_id, int max_nbytes, int chunksize)
{   //TODO fix (must now pass chunksize down)
    void* exmem_out_buffer = ebsp_ext_malloc(max_nbytes);
    if (exmem_out_buffer == 0)
    {
        printf("ERROR: not enough memory in exmem for ebsp_get_buffered\n");
        return;
    }
    state.combuf.exmem_current_out_chunk[dst_core_id] = _arm_to_e_pointer(exmem_out_buffer);
}
*/

