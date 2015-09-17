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


void ebsp_set_up_chunk_size(unsigned stream_id, int nbytes)
{
    ebsp_stream_descriptor* out_stream =
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    int* header = out_stream->current_buffer;
    *header = nbytes;
}


int ebsp_open_up_stream(void** address, unsigned stream_id)
{
    ebsp_stream_descriptor* out_stream =
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    if (out_stream->is_instream)
    {
        ebsp_message("ERROR: tried writing out input stream");
        return 0;
    }

    if (out_stream->current_buffer != NULL)
    {
        ebsp_message("ERROR: tried creating opened stream");
        return 0;
    }

    out_stream->current_buffer =
                        ebsp_malloc(out_stream->max_chunksize + 2*sizeof(int));
    (*address) = (void*) (out_stream->current_buffer + 2*sizeof(int));

    //Set the out_size to max_chunksize
    *((int*)(out_stream->current_buffer)) = out_stream->max_chunksize;

    out_stream->cursor = out_stream->extmem_addr;

    return out_stream->max_chunksize;
}


void ebsp_close_up_stream(unsigned stream_id)
{
    ebsp_stream_descriptor* out_stream =
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    if (out_stream->is_instream)
    {
        ebsp_message("ERROR: tried writing out input stream");
        return;
    }

    e_dma_desc_t* desc = (e_dma_desc_t*) &(out_stream->e_dma_desc);
    ebsp_dma_wait(desc);

    if (out_stream->current_buffer == NULL)
    {
        ebsp_message("ERROR: tried closing closed stream");
        return;
    }

    ebsp_free(out_stream->current_buffer);
    out_stream->current_buffer = NULL;

    if (out_stream->next_buffer != NULL)
    {
        ebsp_free(out_stream->next_buffer);
    }

    out_stream->next_buffer = NULL;
}



int ebsp_move_chunk_up(void** address, unsigned stream_id, int prealloc)
{
    ebsp_stream_descriptor* out_stream =
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    if (out_stream->is_instream)
    {
        ebsp_message("ERROR: tried writing out input stream");
        return 0;
    }

    e_dma_desc_t* desc = (e_dma_desc_t*) &(out_stream->e_dma_desc);

    if (out_stream->next_buffer != NULL) // did prealloc last time
    {
        ebsp_dma_wait(desc);
    }

    if (prealloc)
    {
        if (out_stream->next_buffer == NULL)
        {
            out_stream->next_buffer =
                        ebsp_malloc(out_stream->max_chunksize + 2*sizeof(int));
        }

        // read int header from current_buffer (next size)
        size_t chunk_size = *((int*)(out_stream->current_buffer)); 

        void* tmp = out_stream->current_buffer; //swap buffers
        out_stream->current_buffer = out_stream->next_buffer;
        out_stream->next_buffer = tmp;

        void* src = (out_stream->next_buffer + 2*sizeof(int));
        void* dst = out_stream->cursor;

        ebsp_dma_push(desc, dst, src, chunk_size);  // start dma
        out_stream->cursor += chunk_size; // move pointer in extmem
    }
    else //no prealloc
    {
        if (out_stream->next_buffer != NULL)
        {
            ebsp_free(out_stream->next_buffer);
        }

        // read int header from current_buffer (next size)
        size_t chunk_size = *((int*)(out_stream->current_buffer)); 
        
        void* src = (out_stream->current_buffer + 2*sizeof(int));
        void* dst = out_stream->cursor;

        ebsp_dma_push(desc, dst, src, chunk_size);  // start dma
        out_stream->cursor += chunk_size; // move pointer in extmem
        ebsp_dma_wait(desc);
    }

    (*address) = (void*) (out_stream->current_buffer + 2*sizeof(int));

    //Set the out_size to max_chunksize
    *((int*)(out_stream->current_buffer)) = out_stream->max_chunksize;

    return out_stream->max_chunksize;
}



void ebsp_open_down_stream(unsigned stream_id)
{
    ebsp_stream_descriptor* in_stream =
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    e_dma_desc_t* desc = (e_dma_desc_t*) &(in_stream->e_dma_desc);

    if (! (in_stream->is_instream) ) 
    {
        ebsp_message("ERROR: tried reading from output stream");
        return;
    }
    if (in_stream->current_buffer != NULL || in_stream->next_buffer != NULL)
    {
        ebsp_message("ERROR: tried opening from opened stream");
        return;
    }

    in_stream->cursor = in_stream->extmem_addr;

    in_stream->next_buffer =
                         ebsp_malloc(in_stream->max_chunksize + 2*sizeof(int));

    // read 2nd int in header from ext (next size)
    size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));

    if (chunk_size == 0)    // stream has ended???
    {
        ebsp_message("ERROR: tried opening empty stream");
        return;
    }

    void* dst = in_stream->next_buffer;
    void* src = in_stream->cursor;

    // write to current
    ebsp_dma_push(desc, dst, src, chunk_size + 2*sizeof(int));

    // jump over header+chunk
    in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
            + 2*sizeof(int) + chunk_size); 
}

void ebsp_close_down_stream(unsigned stream_id)
{
    ebsp_stream_descriptor* in_stream = 
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    e_dma_desc_t* desc = (e_dma_desc_t*) &(in_stream->e_dma_desc);

    ebsp_dma_wait(desc);

    if (! (in_stream->is_instream) ) 
    {
        ebsp_message("ERROR: tried reading from output stream");
        return;
    }
    if (in_stream->current_buffer == NULL)
    {
        ebsp_message("ERROR: tried closing closed stream");
        return;
    }
 
    ebsp_free(in_stream->current_buffer);
    in_stream->current_buffer = NULL;

    if(in_stream->next_buffer != NULL) {
        ebsp_free(in_stream->next_buffer);
        in_stream->next_buffer = NULL;
    }
}


int ebsp_move_chunk_down(void** address, unsigned stream_id, int prealloc)
{

    ebsp_stream_descriptor* in_stream =
             coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);

    e_dma_desc_t* desc = (e_dma_desc_t*) &(in_stream->e_dma_desc);

    if(in_stream -> current_buffer == NULL)
        in_stream -> current_buffer =
                         ebsp_malloc(in_stream->max_chunksize + 2*sizeof(int));

    if (! (in_stream->is_instream) ) 
    {
        ebsp_message("ERROR: tried reading from output stream");
        return 0;
    }

    if (in_stream->next_buffer == NULL) // did not prealloc last time
    {
        // read 2nd int in header from ext (next size)
        size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int)); 

        if (chunk_size != 0)    // stream has not ended
        {
            void* dst = in_stream->current_buffer;
            void* src = in_stream->cursor;

            // write to current
            ebsp_dma_push(desc, dst, src, chunk_size + 2*sizeof(int));

            // jump over header+chunk
            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                 + 2*sizeof(int) + chunk_size); 
        } else {
            // set next size to 0
            *((int*)(in_stream->current_buffer + sizeof(int))) = 0;
        }
    } 
    else // did prealloc last time
    { 
        void* tmp = in_stream->current_buffer;
        in_stream->current_buffer = in_stream->next_buffer;
        in_stream->next_buffer = tmp;
    }

    // *address must point after the counter header
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
            in_stream->next_buffer =
                         ebsp_malloc(in_stream->max_chunksize + 2*sizeof(int));

        // read 2nd int in (next size) header from ext
        size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));

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


void ebsp_reset_down_cursor(int stream_id)
{
    ebsp_stream_descriptor* in_stream = 
            coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);
    size_t chunk_size = -1;

    // break when previous block has size 0 (begin of stream)
    while(chunk_size != 0) {
        // read 1st int in (prev size) header from ext
        chunk_size = *(int*)(in_stream->cursor);  
        in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                - 2*sizeof(int) - chunk_size);
    }
}


void ebsp_move_down_cursor(int stream_id, int jump_n_chunks) {
    ebsp_stream_descriptor* in_stream =
             coredata.local_streams + stream_id*sizeof(ebsp_stream_descriptor);
    
    if (jump_n_chunks > 0) //jump forward
    {
        while (jump_n_chunks--)
        {
            // read 2nd int in (next size) header from ext
            size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));  
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
            // read 1st int in (prev size) header from ext
            size_t chunk_size = *(int*)(in_stream->cursor);  
            if (chunk_size == 0) {
                ebsp_message("ERROR: tried to jump to before the first chunk");
                return;
            }
            in_stream->cursor = (void*) (((unsigned) (in_stream->cursor))
                                                 - 2*sizeof(int) - chunk_size);
        }
    }
}



