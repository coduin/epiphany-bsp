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

const char err_no_such_stream2[] EXT_MEM_RO = "BSP ERROR: stream does not exist";

const char err_out_of_memory2[] EXT_MEM_RO =
    "BSP ERROR: could not allocate enough memory for stream";

const char err_stream_in_use[] EXT_MEM_RO =
    "BSP ERROR: stream with id %d is in use";

const char err_stream_full[] EXT_MEM_RO =
    "BSP ERROR: Stream %d has %u space left, token of size %u can not be moved up.";

const char err_up_size_warning[] EXT_MEM_RO =
    "BSP WARNING: Moving token of size %d up to stream %d with max token size %d";

const char err_token_size[] EXT_MEM_RO =
    "BSP ERROR: Stream contained token larger (%d) than maximum token size (%d) for stream. (truncated)";

void _ebsp_read_chunk(ebsp_stream* stream, void* target) {
    // read header from ext
    int prev_size = *(int*)(stream->cursor);
    int chunk_size = *(int*)(stream->cursor + sizeof(int));

    if (chunk_size != 0) // stream has not ended
    {
        void* dst = target + 2 * sizeof(int);
        void* src = stream->cursor + 2 * sizeof(int);

        // jump over header+chunk
        stream->cursor = (void*)(((unsigned)(stream->cursor)) +
                                 2 * sizeof(int) + chunk_size);

        // If token is too large, truncate it.
        // However DO jump the correct distance with cursor
        if (chunk_size > stream->max_chunksize) {
            ebsp_message(err_token_size, chunk_size, stream->max_chunksize);
            chunk_size = stream->max_chunksize;
        }

        ebsp_dma_push(&stream->e_dma_desc, dst, src, chunk_size);
    }

    // copy it to local
    // we do NOT do this with the DMA because of the
    // possible trunction done above
    *(int*)(target) = prev_size;
    *(int*)(target + sizeof(int)) = chunk_size;
}

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

int bsp_stream_open(ebsp_stream* stream, int stream_id) {
    if (stream_id >= combuf->nstreams) {
        ebsp_message(err_no_such_stream2);
        return 0;
    }
    ebsp_stream_descriptor* s = &(combuf->streams[stream_id]);

    int mypid = coredata.pid;

    e_mutex_lock(0, 0, &coredata.stream_mutex);
    if (s->pid == -1) {
        s->pid = mypid;
        mypid = -1;
    }
    e_mutex_unlock(0, 0, &coredata.stream_mutex);

    if (mypid != -1) {
        ebsp_message(err_stream_in_use, stream_id);
        return 0;
    }

    // Fill stream struct
    stream->id = stream_id;
    stream->extmem_start = s->extmem_addr;
    stream->extmem_end = stream->extmem_start + s->nbytes;
    stream->current_buffer = NULL;
    stream->next_buffer = NULL;
    stream->max_chunksize = s->max_chunksize;

    // Go to start
    stream->cursor = stream->extmem_start;

    return stream->max_chunksize;
}

void bsp_stream_close(ebsp_stream* stream) {
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

    // Should not have to lock mutex for this atomic write
    combuf->streams[stream->id].pid = -1;
    stream->id = -1;
}

void bsp_stream_seek(ebsp_stream* stream, int delta_tokens) {
    if (delta_tokens >= 0) { // forward
        while (delta_tokens--) {
            // read 2nd int (next size) in header
            int chunk_size = *(int*)(stream->cursor + sizeof(int));
            if (chunk_size == 0)
                break;
            stream->cursor += 2 * sizeof(int) + chunk_size;
        }
    } else { // backward
        if (delta_tokens == INT_MIN) {
            stream->cursor = stream->extmem_start;
        } else {
            while (delta_tokens++) {
                // read 1st int (prev size) in header
                int chunk_size = *(int*)(stream->cursor);
                if (chunk_size == 0)
                    break;
                stream->cursor -= 2 * sizeof(int) + chunk_size;
            }
        }
    }

    // If there was anything preloaded, discard it
    if (stream->next_buffer != NULL) {
        // Wait for a possible write to it
        ebsp_dma_wait(&stream->e_dma_desc);
        // Free it
        ebsp_free(stream->next_buffer);
        stream->next_buffer = NULL;
    }
}

int bsp_stream_move_down(ebsp_stream* stream, void** buffer, int preload) {
    *buffer = NULL;

    if (stream->current_buffer == NULL) {
        stream->current_buffer =
            ebsp_malloc(stream->max_chunksize + 2 * sizeof(int));
        if (stream->current_buffer == NULL) {
            ebsp_message(err_out_of_memory2);
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
        _ebsp_read_chunk(stream, stream->current_buffer);
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
                ebsp_message(err_out_of_memory2);
                return 0;
            }
        }
        _ebsp_read_chunk(stream, stream->next_buffer);
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

int bsp_stream_move_up(ebsp_stream* stream, const void* data, int data_size,
                        int wait_for_completion) {
    ebsp_dma_handle* desc = &stream->e_dma_desc;

    // Wait for any previous transfer to finish (either down or up)
    ebsp_dma_wait(desc);

    // Round data_size up to a multiple of 8
    // If this is not done, integer access to the headers will crash
    data_size = ((data_size + 8 - 1) / 8) * 8;

    if (data_size > stream->max_chunksize) {
        ebsp_message(err_up_size_warning, data_size, stream->id,
                     stream->max_chunksize);
    }

    // Check if there is enough space in the stream,
    // including terminating header
    // Be carefull to use unsigned here, since these addresses
    // are over the INT_MAX boundary.
    unsigned space_required = (unsigned)data_size + 4 * sizeof(int);
    unsigned space_left = (unsigned)stream->extmem_end - (unsigned)stream->cursor;
    if (space_left < space_required) {
        ebsp_message(err_stream_full, stream->id, space_left, space_required);
        return 0;
    }

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

