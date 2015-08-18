/*
File: e_bsp_buffer.c

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

#include "e_bsp_private.h"
#include <string.h>


int get_next_chunk(void** address, unsigned stream_id, int prealloc)
{
    ebsp_message("get_next_chunk");
    ebsp_in_stream_descriptor* in_stream = coredata.local_in_streams + stream_id*sizeof(ebsp_in_stream_descriptor);
    e_dma_desc_t* desc = (e_dma_desc_t*) &(in_stream->e_dma_desc);
    ebsp_message("desc = %p", desc);

    if (in_stream->next_in_buffer == NULL) // did not prealloc last time
    {
        if (in_stream->current_in_buffer == NULL) // first time get_next_chunk is called!
            in_stream->current_in_buffer = ebsp_malloc(in_stream->max_chunksize);

        size_t chunk_size = *(int*)(in_stream->in_cursor);  // read header from ext
        void* dst = in_stream->current_in_buffer;
        void* src = in_stream->in_cursor;
        ebsp_message("NOW pushing dma %p <- %p, size %d", dst, src, chunk_size);
        ebsp_dma_push(desc, dst, src, chunk_size + sizeof(int));  // write to current

        // jump over header+chunk

        ebsp_message("NOW JUMP FR %p ", in_stream->in_cursor);
        in_stream->in_cursor = (void*) (((unsigned) (in_stream->in_cursor))
                                                 + sizeof(int) + chunk_size); 
        ebsp_message("NOW JUMP TO %p ", in_stream->in_cursor);
    } 
    else // did prealloc last time
    { 
        void* tmp = in_stream->current_in_buffer;
        in_stream->current_in_buffer = in_stream->next_in_buffer;
        in_stream->next_in_buffer = tmp;
    }

    // *address points after the counter header
    (*address) = (void*) ((unsigned)in_stream->current_in_buffer+sizeof(int));
    
    ebsp_message("waiting for dma");
    ebsp_dma_wait(desc);
    ebsp_message("done waiting for dma");

    // the counter header
    int current_chunk_size = *((int*)in_stream->current_in_buffer); 
  
    if (coredata.pid == 0)
    { 
        ebsp_message(">>>\t\t\t\t\t\t\t\t\t>>>>>>>> current_chunk_size = %d = %d * %d", current_chunk_size, sizeof(int), current_chunk_size/sizeof(int));
        for (int i=0; i<current_chunk_size/(sizeof(int)); i++)
        {
            unsigned cursor = (unsigned)(*address) + i*sizeof(int);
            ebsp_message(">>>\t\t\t\t\t\t\t\t\t>>>>>>>> %03d: %d", i, *((int*)cursor));
        }
    }
        
    if (current_chunk_size == 0)    // stream has ended
    {
        (*address) = NULL;
        return 0;
    }
     
    if (prealloc)
    {
        if (in_stream->next_in_buffer == NULL)
            in_stream->next_in_buffer = ebsp_malloc(in_stream->max_chunksize);

        size_t chunk_size = *(int*)(in_stream->in_cursor);  // read header from ext
        void* dst = in_stream->next_in_buffer;
        void* src = in_stream->in_cursor;
        ebsp_message("PRE pushing dma %p <- %p, size %d", dst, src, chunk_size + sizeof(int));
        ebsp_dma_push(desc, dst, src, chunk_size);  // write to next

        // jump over header+chunk
        ebsp_message("PRE JUMP FR %p ", in_stream->in_cursor);
        in_stream->in_cursor = (void*) (((unsigned) (in_stream->in_cursor))
                                                 + sizeof(int) + chunk_size); 
        ebsp_message("PRE JUMP TO %p ", in_stream->in_cursor);
    }
    else
    {
        if (in_stream->next_in_buffer != NULL)
        {
            ebsp_free(in_stream->next_in_buffer);
            in_stream->next_in_buffer = NULL;
        }

    }

    return current_chunk_size;
}

/*
void* ebsp_get_in_chunk() {//TODO rewrite
    coredata.exmem_next_in_chunk += IN_CHUNK_SIZE;

    void* tmp = coredata.buffer_in_current;
    coredata.buffer_in_current = coredata.buffer_in_next;
    coredata.buffer_in_next    = tmp;

    ebsp_dma_copy_parallel( E_DMA_0, coredata.buffer_in_next, coredata.exmem_next_in_chunk, (size_t) IN_CHUNK_SIZE );//REPLACE BY QUEUE
    return coredata.buffer_in_current;
}

void* ebsp_get_out_chunk() {
    coredata.exmem_current_out_chunk += OUT_CHUNK_SIZE;//TODO Change OUT_CHUNK_SIZE to var passed down 
    //FIXME check for overflow

    void* tmp = coredata.buffer_out_current;//TODO support slow mode?
    coredata.buffer_out_current  = coredata.buffer_out_previous;
    coredata.buffer_out_previous = tmp;

    ebsp_dma_copy_parallel( E_DMA_1, coredata.exmem_current_out_chunk, coredata.buffer_out_previous, (size_t) OUT_CHUNK_SIZE );//TODO REPLACE BY QUEUE

    return coredata.buffer_out_current;
}
*/
