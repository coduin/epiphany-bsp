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

#include "e_bsp_private.h"
#include <limits.h>

const char err_no_such_stream[] EXT_MEM_RO = "BSP ERROR: stream does not exist";

const char err_mixed_up_down[] EXT_MEM_RO =
    "BSP ERROR: mixed up and down streams";

const char err_close_closed[] EXT_MEM_RO =
    "BSP ERROR: tried to close closed stream";

const char err_open_opened[] EXT_MEM_RO =
    "BSP ERROR: tried to open opened stream";

const char err_jump_out_of_bounds[] EXT_MEM_RO =
    "BSP ERROR: tried jumping past bounds of stream";

const char err_create_opened[] EXT_MEM_RO =
    "BSP ERROR: tried creating opened stream";

const char err_out_of_memory[] EXT_MEM_RO =
    "BSP ERROR: could not allocate enough memory for stream";

void ebsp_set_up_chunk_size(unsigned stream_id, int nbytes) {
    ebsp_stream_descriptor* out_stream = &coredata.local_streams[stream_id];

    int* header = (int*)out_stream->current_buffer;
    // update the *next* value to the new numer of bytes
    header[1] = nbytes;
}

int ebsp_open_up_stream(void** address, unsigned stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    if (stream->is_down_stream) {
        ebsp_message(err_mixed_up_down);
        return 0;
    }

    if (stream->current_buffer != NULL) {
        ebsp_message(err_create_opened);
        return 0;
    }

    stream->current_buffer = ebsp_malloc(stream->max_chunksize + sizeof(int));
    if (stream->current_buffer == NULL) {
        ebsp_message(err_out_of_memory);
        return 0;
    }

    (*address) = (void*)((unsigned)stream->current_buffer + sizeof(int));

    // Set the size to max_chunksize
    int* header = (int*)stream->current_buffer;
    header[0] = stream->max_chunksize;

    stream->cursor = stream->extmem_addr;

    return stream->max_chunksize;
}

void ebsp_close_up_stream(unsigned stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }

    ebsp_stream_descriptor* out_stream = &coredata.local_streams[stream_id];

    if (out_stream->is_down_stream) {
        ebsp_message(err_mixed_up_down);
        return;
    }

    if (out_stream->current_buffer == NULL) {
        ebsp_message(err_close_closed);
        return;
    }

    // wait for data transfer to finish before closing
    ebsp_dma_handle* desc = (ebsp_dma_handle*)&out_stream->e_dma_desc;
    ebsp_dma_wait(desc);

    ebsp_free(out_stream->current_buffer);
    out_stream->current_buffer = NULL;

    if (out_stream->next_buffer != NULL) {
        ebsp_free(out_stream->next_buffer);
        out_stream->next_buffer = NULL;
    }
}

int ebsp_move_chunk_up(void** address, unsigned stream_id, int prealloc) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    if (stream->is_down_stream) {
        ebsp_message(err_mixed_up_down);
        return 0;
    }

    ebsp_dma_handle* desc = (ebsp_dma_handle*)&stream->e_dma_desc;

    // if we prealloced last time, we have to wait until dma is finished
    if (stream->next_buffer != NULL) {
        ebsp_dma_wait(desc);
    }

    if (prealloc) {
        if (stream->next_buffer == NULL) {
            stream->next_buffer =
                ebsp_malloc(stream->max_chunksize + sizeof(int));
            if (stream->next_buffer == NULL) {
                ebsp_message(err_out_of_memory);
                return 0;
            }
        }

        // read int header from current_buffer (next size)
        int chunk_size = ((int*)stream->current_buffer)[0];

        void* src = (void*)((unsigned)stream->current_buffer + sizeof(int));
        void* dst = stream->cursor;

        ebsp_dma_push(desc, dst, src, chunk_size); // start dma
        // ebsp_dma_start();

        void* tmp = stream->current_buffer; // swap buffers
        stream->current_buffer = stream->next_buffer;
        stream->next_buffer = tmp;

        stream->cursor += chunk_size; // move pointer in extmem
    } else                            // no prealloc
    {
        if (stream->next_buffer != NULL) {
            ebsp_free(stream->next_buffer);
            stream->next_buffer = NULL;
        }

        // read int header from current_buffer (next size)
        int chunk_size = ((int*)stream->current_buffer)[0];

        void* src = (void*)((unsigned)stream->current_buffer + sizeof(int));
        void* dst = stream->cursor;

        ebsp_dma_push(desc, dst, src, chunk_size); // start dma
        // ebsp_dma_start();
        ebsp_dma_wait(desc);

        stream->cursor += chunk_size; // move pointer in extmem
    }

    (*address) = (void*)((unsigned)stream->current_buffer + sizeof(int));

    // Set the out_size to max_chunksize
    *((int*)(stream->current_buffer)) = stream->max_chunksize;

    return stream->max_chunksize;
}

void _ebsp_write_chunk(ebsp_stream_descriptor* stream, void* target) {
    // read 2nd int in header from ext (next size)
    int chunk_size = *(int*)(stream->cursor + sizeof(int));
    ebsp_dma_handle* desc = (ebsp_dma_handle*)&(stream->e_dma_desc);

    if (chunk_size != 0) // stream has not ended
    {
        void* dst = target;
        void* src = stream->cursor;

        // write to current
        ebsp_dma_push(desc, dst, src, chunk_size + 2 * sizeof(int));
        // ebsp_dma_start();

        // jump over header+chunk
        stream->cursor = (void*)(((unsigned)(stream->cursor)) +
                                 2 * sizeof(int) + chunk_size);
    } else {
        // set next size to 0
        *((int*)(target + sizeof(int))) = 0;
    }
}

int ebsp_open_down_stream(void** address, unsigned stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    if (!stream->is_down_stream) {
        ebsp_message(err_mixed_up_down);
        return 0;
    }
    if (stream->current_buffer != NULL || stream->next_buffer != NULL) {
        ebsp_message(err_open_opened);
        return 0;
    }

    stream->cursor = stream->extmem_addr;

    // this will be the current buffer when move_chunk_down gets called for
    // the first time
    stream->next_buffer = ebsp_malloc(stream->max_chunksize + 2 * sizeof(int));
    if (stream->next_buffer == NULL) {
        ebsp_message(err_out_of_memory);
        return 0;
    }

    _ebsp_write_chunk(stream, stream->next_buffer);

    *address = (void*)((unsigned)stream->next_buffer + 2 * sizeof(int));

    return stream->max_chunksize;
}

void ebsp_close_down_stream(unsigned stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }

    ebsp_stream_descriptor* in_stream = &coredata.local_streams[stream_id];

    if (!(in_stream->is_down_stream)) {
        ebsp_message(err_mixed_up_down);
        return;
    }

    if (in_stream->current_buffer == NULL) {
        ebsp_message(err_close_closed);
        return;
    }

    ebsp_dma_handle* desc = (ebsp_dma_handle*)&in_stream->e_dma_desc;
    ebsp_dma_wait(desc);

    ebsp_free(in_stream->current_buffer);
    in_stream->current_buffer = NULL;

    if (in_stream->next_buffer != NULL) {
        ebsp_free(in_stream->next_buffer);
        in_stream->next_buffer = NULL;
    }
}

int ebsp_move_chunk_down(void** address, unsigned stream_id, int prealloc) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    ebsp_dma_handle* desc = (ebsp_dma_handle*)(&(stream->e_dma_desc));

    // if(stream->current_buffer == NULL)
    //    stream->current_buffer =
    //                     ebsp_malloc(stream->max_chunksize + 2*sizeof(int));

    // Here: current_buffer contains data from previous chunk
    // this can be null the first time ebsp_move_chunk_down is called

    if (!(stream->is_down_stream)) {
        ebsp_message(err_mixed_up_down);
        return 0;
    }

    if (stream->next_buffer == NULL) // did not prealloc last time
    {
        // overwrite current buffer
        _ebsp_write_chunk(stream, stream->current_buffer);
    } else // did prealloc last time
    {
        void* tmp = stream->current_buffer;
        stream->current_buffer = stream->next_buffer;
        stream->next_buffer = tmp;
    }

    // either wait for dma_push from last prealloc (else)
    // or the one we just started (if)
    ebsp_dma_wait(desc);

    // Here: current_buffer contains data from THIS chunk

    // *address must point after the counter header
    (*address) = (void*)((unsigned)stream->current_buffer + 2 * sizeof(int));

    // the counter header
    int current_chunk_size =
        *((int*)((unsigned)stream->current_buffer + sizeof(int)));

    if (current_chunk_size == 0) // stream has ended
    {
        (*address) = NULL;
        return 0;
    }

    if (prealloc) {
        if (stream->next_buffer == NULL) {
            // no next buffer available, malloc it
            stream->next_buffer =
                ebsp_malloc(stream->max_chunksize + 2 * sizeof(int));
            if (stream->next_buffer == NULL) {
                ebsp_message(err_out_of_memory);
                return 0;
            }
        }
        _ebsp_write_chunk(stream, stream->next_buffer);
    } else {
        // free malloced next buffer
        if (stream->next_buffer != NULL) {
            ebsp_free(stream->next_buffer);
            stream->next_buffer = NULL;
        }
    }

    // Here: next_buffer should (possibly) point to data of NEXT
    // chunk (begin written to) or be zero

    return current_chunk_size;
}

void ebsp_reset_down_cursor(int stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }

    ebsp_stream_descriptor* in_stream = &coredata.local_streams[stream_id];

    size_t chunk_size = -1;

    // break when previous block has size 0 (begin of stream)
    while (chunk_size != 0) {
        // read 1st int in (prev size) header from ext
        chunk_size = *(int*)(in_stream->cursor);
        in_stream->cursor = (void*)(((unsigned)(in_stream->cursor)) -
                                    2 * sizeof(int) - chunk_size);
    }
}

void ebsp_move_down_cursor(int stream_id, int jump_n_chunks) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }

    ebsp_stream_descriptor* in_stream = &coredata.local_streams[stream_id];

    if (jump_n_chunks > 0) // jump forward
    {
        while (jump_n_chunks--) {
            // read 2nd int in (next size) header from ext
            size_t chunk_size = *(int*)(in_stream->cursor + sizeof(int));
            if (chunk_size == 0) {
                ebsp_message(err_jump_out_of_bounds);
                return;
            }
            in_stream->cursor = (void*)(((unsigned)(in_stream->cursor)) +
                                        2 * sizeof(int) + chunk_size);
        }
    } else // jump backward
    {
        while (jump_n_chunks++) {
            // read 1st int in (prev size) header from ext
            int chunk_size = *(int*)(in_stream->cursor);
            if (chunk_size == 0) {
                ebsp_message(err_jump_out_of_bounds);
                return;
            }
            in_stream->cursor = (void*)(((unsigned)(in_stream->cursor)) -
                                        2 * sizeof(int) - chunk_size);
        }
    }
}

//
// New streaming API starting here
//

// When stream headers are interleaved, they are saved as:
//
// 00000000, nextsize, data,
// prevsize, nextsize, data,
// ...
// prevsize, nextsize, data,
// prevsize, 00000000
//
// So a header consists of two integers (8 byte total).
// The two sizes do NOT include these headers.
// They are only the size of the data inbetween.
// The local copies of the data include these 8 bytes.

int ebsp_stream_open(int stream_id, ebsp_stream* stream) {
    if (stream_id >= combuf->nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }
    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    // Go to start
    stream->cursor = stream->extmem_addr;

    return stream->max_chunksize;
}

void ebsp_stream_close(int stream_id) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }
    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    // Wait for any data transfer to finish before closing
    ebsp_dma_wait(&stream->e_dma_desc);

    if (stream->current_buffer != NULL) {
        ebsp_free(stream->current_buffer);
        stream->current_buffer = NULL;
    }
    if (stream->next_buffer != NULL) {
        ebsp_free(stream->next_buffer);
        stream->next_buffer = NULL;
    }
}

void ebsp_stream_seek(int stream_id, int delta_tokens) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return;
    }
    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    if (delta_tokens >= 0) { // forward
        while (delta_tokens--) {
            // read 2nd int (next size) in header
            int chunk_size = *(int*)(stream->cursor + sizeof(int));
            if (chunk_size == 0)
                return;
            stream->cursor += 2 * sizeof(int) + chunk_size;
        }
    } else { // backward
        if (delta_tokens == INT_MIN) {
            stream->cursor = stream->extmem_addr;
        }

        while (delta_tokens++) {
            // read 1st int (prev size) in header
            int chunk_size = *(int*)(stream->cursor);
            if (chunk_size == 0)
                return;
            stream->cursor -= 2 * sizeof(int) + chunk_size;
        }
    }
}

int ebsp_stream_move_down(int stream_id, void** buffer, int preload) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    if (stream->current_buffer == NULL) {
        stream->current_buffer =
            ebsp_malloc(stream->max_chunksize + 2 * sizeof(int));
        if (stream->current_buffer == NULL) {
            ebsp_message(err_out_of_memory);
            return 0;
        }
    }

    // Wait for any previous transfer to finish (either down or up)
    ebsp_dma_wait(&(stream->e_dma_desc));

    // At this point in the code:
    //  current_buffer contains data from previous token,
    //  which has been given to the user last time (zero at first time).
    // This means current_buffer can be overwritten now
    // The new token is:
    //  - locally available already in next_buffer (preload)
    //  - not here yet (no preload)

    if (stream->next_buffer == NULL) {
        // Data not here yet (did not preload last time)
        // Overwrite current buffer.
        _ebsp_write_chunk(stream, stream->current_buffer);
        ebsp_dma_wait(&(stream->e_dma_desc));
    } else {
        // Data is locally available already in next_buffer (preload).
        // Swap buffers.
        void* tmp = stream->current_buffer;
        stream->current_buffer = stream->next_buffer;
        stream->next_buffer = tmp;
    }

    // At this point in the code:
    //  current_buffer contains data from the current token,
    //  which we should now give to the user.

    // *buffer must point after the header
    (*buffer) = (void*)((unsigned)stream->current_buffer + 2 * sizeof(int));

    int* header = (int*)(stream->current_buffer);
    int current_chunk_size = header[1];

    // Check for end-of-stream
    if (current_chunk_size == 0) {
        (*buffer) = NULL;
        return 0;
    }

    if (preload) {
        if (stream->next_buffer == NULL) {
            // no next buffer available, malloc it
            stream->next_buffer =
                ebsp_malloc(stream->max_chunksize + 2 * sizeof(int));
            if (stream->next_buffer == NULL) {
                ebsp_message(err_out_of_memory);
                return 0;
            }
        }
        _ebsp_write_chunk(stream, stream->next_buffer);
    } else {
        // free malloced next buffer
        if (stream->next_buffer != NULL) {
            ebsp_free(stream->next_buffer);
            stream->next_buffer = NULL;
        }
    }

    // At this point: next_buffer should point to data of NEXT token

    return current_chunk_size;
}

int ebsp_stream_move_up(int stream_id, const void* data, int data_size,
                        int wait_for_completion) {
    if (stream_id >= coredata.local_nstreams) {
        ebsp_message(err_no_such_stream);
        return 0;
    }

    ebsp_stream_descriptor* stream = &coredata.local_streams[stream_id];

    ebsp_dma_handle* desc = &stream->e_dma_desc;

    // Wait for any previous transfer to finish (either down or up)
    ebsp_dma_wait(desc);

    // Round data_size up to a multiple of 8
    // If this is not done, integer access to the headers will crash
    data_size = ((data_size + 8 - 1) / 8) * 8;

    // First write both the header before and after this token.
    int* header1 = (int*)(stream->cursor);
    int* header2 = (int*)(stream->cursor + 2 * sizeof(int) + data_size);
    // header1[0] is filled at the previous iteration
    header1[1] = data_size;
    header2[0] = data_size;
    header2[1] = 0; // terminating 0

    stream->cursor += 2 * sizeof(int);

    // Now write the data to extmem (async)
    ebsp_dma_push(desc, (void*)(stream->cursor), data, data_size); // start dma
    stream->cursor += data_size; // move pointer in extmem

    if (wait_for_completion)
        ebsp_dma_wait(desc);

    return data_size;
}

