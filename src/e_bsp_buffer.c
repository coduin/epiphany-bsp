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


int ebsp_get_next_chunk(void** address, unsigned stream_id, int prealloc)
{
    ebsp_stream_descriptor* in_stream = coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    e_dma_desc_t* desc = (e_dma_desc_t*) &(in_stream->e_dma_desc);

    if (! (in_stream->is_instream) ) 
    {
        ebsp_message("ERROR: tried reading from output stream");
    }

    if (in_stream->next_buffer == NULL) // did not prealloc last time
    {
        if (in_stream->current_buffer == NULL) // first time get_next_chunk is called!
            in_stream->current_buffer = ebsp_malloc(in_stream->max_chunksize + 2*sizeof(int));

        size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int)); // read 2nd int in header from ext (next size)

        if (chunk_size != 0)    // stream has not ended
        {
            void* dst = in_stream->current_buffer;
            void* src = in_stream->cursor;
            ebsp_dma_push(desc, dst, src, chunk_size + 2*sizeof(int));  // write to current

            // jump over header+chunk

            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                     + 2*sizeof(int) + chunk_size); 
        } else {
            *((int*)(in_stream->current_buffer + sizeof(int))) = 0; // set next size to 0
        }
    } 
    else // did prealloc last time
    { 
        void* tmp = in_stream->current_buffer;
        in_stream->current_buffer = in_stream->next_buffer;
        in_stream->next_buffer = tmp;
    }

    // *address points after the counter header
    (*address) = (void*) ((unsigned)in_stream->current_buffer + 2*sizeof(int));
    
    ebsp_dma_wait(desc);

    // the counter header
    int current_chunk_size = *((int*)(in_stream->current_buffer + sizeof(int))); 
  
    if (current_chunk_size == 0)    // stream has ended
    {
        (*address) = NULL;
        return 0;
    }
     
    if (prealloc)
    {
        if (in_stream->next_buffer == NULL)
            in_stream->next_buffer = ebsp_malloc(in_stream->max_chunksize + 2*sizeof(int));

        size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));  // read 2nd int in (next size) header from ext

        if (chunk_size != 0)    // stream has not ended
        {
            void* dst = in_stream->next_buffer;
            void* src = in_stream->cursor;
            ebsp_dma_push(desc, dst, src, chunk_size);  // write to next

            // jump over header+chunk
            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                     + 2*sizeof(int) + chunk_size); 
        } else {
            *((int*)(in_stream->next_buffer + sizeof(int))) = 0;
        }
    }
    else
    {
        if (in_stream->next_buffer != NULL)
        {
            ebsp_free(in_stream->next_buffer);
            in_stream->next_buffer = NULL;
        }

    }

    return current_chunk_size;
}



void ebsp_move_in_cursor(int stream_id, int jump_n_chunks) {
    ebsp_stream_descriptor* in_stream = coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);
    
    if (jump_n_chunks > 0) //jump forward
    {
        while (jump_n_chunks--)
        {
            size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));  // read 2nd int in (next size) header from ext
            if (chunk_size == 0) {
                ebsp_message("ERROR: tried to jump to after the last chunk");
                return;
            }
            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                 + 2*sizeof(int) + chunk_size); 
        }
    }
    else //jump backward
    {
        while (jump_n_chunks++)
        {
            size_t chunk_size = *(int*)(in_stream->cursor);  // read 1st int in (prev size) header from ext
            if (chunk_size == 0) {
                ebsp_message("ERROR: tried to jump to before the first chunk");
                return;
            }
            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                 - 2*sizeof(int) - chunk_size);
        }
    }
}






/*
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
