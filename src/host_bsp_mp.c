/*
File: host_bsp_mp.c

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

void ebsp_set_tagsize(int *tag_bytes)
{
    int oldsize = state.combuf.tagsize;
    state.combuf.tagsize = *tag_bytes;
    *tag_bytes = oldsize;
}

//TODO: Do not use the local copy state.combuf
//Instead, copy directly to the memory mapped external memory

// Convert pointers pointing to the local copy state.combuf
// to epiphany address space and back

void* _pointer_to_e(void* ptr)
{
    return (void*)((unsigned int)ptr
            - (unsigned)&state.combuf
            + E_COMBUF_ADDR);
}

void* _pointer_to_arm(void* ptr)
{
    return (void*)((unsigned int)ptr
            - E_COMBUF_ADDR
            + (unsigned)&state.combuf);
}

void ebsp_send_down(int pid, const void *tag, const void *payload, int nbytes)
{
    ebsp_message_queue* q = &state.combuf.message_queue[0];
    unsigned int index = q->count;
    unsigned int payload_offset = state.combuf.data_payloads.buffer_size;
    unsigned int total_nbytes = state.combuf.tagsize + nbytes;
    void *tag_ptr;
    void *payload_ptr;

    if (index >= MAX_MESSAGES) {
        fprintf(stderr,
                "ERROR: Maximal message count reached in ebsp_send_down.\n");
        return;
    }
    if (payload_offset + total_nbytes > MAX_PAYLOAD_SIZE) {
        fprintf(stderr,
                "ERROR: Maximal data payload sent in ebsp_send_down.\n");
        return;
    }

    q->count++;
    state.combuf.data_payloads.buffer_size += total_nbytes;

    tag_ptr = &state.combuf.data_payloads.buf[payload_offset];
    payload_offset += state.combuf.tagsize;
    payload_ptr = &state.combuf.data_payloads.buf[payload_offset];

    q->message[index].pid = pid;
    q->message[index].tag = _pointer_to_e(tag_ptr);
    q->message[index].payload = _pointer_to_e(payload_ptr);
    q->message[index].nbytes = nbytes;
    memcpy(tag_ptr, tag, state.combuf.tagsize);
    memcpy(payload_ptr, payload, nbytes);
}

int ebsp_get_tagsize()
{
    return state.combuf.tagsize;
}

void ebsp_qsize(int *packets, int *accum_bytes)
{
    *packets = 0;
    *accum_bytes = 0;

    ebsp_message_queue* q = &state.combuf.message_queue[0];
    int mindex = state.message_index;
    int qsize = q->count;

    // Count everything after mindex
    for (; mindex < qsize; mindex++)
    {
        *packets += 1;
        *accum_bytes += q->message[mindex].nbytes;
    }
    return;
}

ebsp_message_header* _next_queue_message()
{
    ebsp_message_queue* q = &state.combuf.message_queue[0];
    if (state.message_index < q->count)
        return &q->message[state.message_index];
    return 0;
}

void _pop_queue_message()
{
    state.message_index++;
}

void ebsp_get_tag(int *status, void *tag)
{
    ebsp_message_header* m = _next_queue_message();
    if (m == 0)
    {
        *status = -1;
        return;
    }
    *status = m->nbytes;
    memcpy(tag, _pointer_to_arm(m->tag), state.combuf.tagsize);
}

void ebsp_move(void *payload, int buffer_size)
{
    ebsp_message_header* m = _next_queue_message();
    _pop_queue_message();
    if (m == 0)
    {
        // This part is not defined by the BSP standard
        return;
    }

    if (buffer_size == 0)  // Specified by BSP standard
        return;

    if (m->nbytes < buffer_size)
        buffer_size = m->nbytes;

    memcpy(payload, _pointer_to_arm(m->payload), buffer_size);
}

int ebsp_hpmove(void **tag_ptr_buf, void **payload_ptr_buf)
{
    ebsp_message_header* m = _next_queue_message();
    _pop_queue_message();
    if (m == 0) return -1;
    *tag_ptr_buf = _pointer_to_arm(m->tag);
    *payload_ptr_buf = _pointer_to_arm(m->payload);
    return m->nbytes;
}

void ebsp_send_buffered(void* src, int dst_core_id, int nbytes)
{

    void* exmem_in_buffer = ebsp_ext_malloc(nbytes);
    if (exmem_in_buffer == 0)
    {
        printf("ERROR: not enough memory in exmem for ebsp_send_buffered");
        return;
    }
    memcpy(src, exmem_in_buffer, nbytes);
    state.combuf.exmem_next_in_chunk[dst_core_id] = exmem_in_buffer;
}

void ebsp_get_buffered(int dst_core_id, int max_nbytes)
{
    void* exmem_out_buffer = ebsp_ext_malloc(max_nbytes);
    if (exmem_out_buffer == 0)
    {
        printf("ERROR: not enough memory in exmem for ebsp_get_buffered");
        return;
    }
    state.combuf.exmem_current_out_chunk[dst_core_id] = exmem_out_buffer;
}


