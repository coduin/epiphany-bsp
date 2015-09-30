/*
File: e_bsp_mp.c

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

const char err_send_overflow[] EXT_MEM_RO =
    "BSP ERROR: too many bsp_send requests per sync";

int ebsp_get_tagsize()
{
    return coredata.tagsize;
}

void EXT_MEM_TEXT bsp_set_tagsize(int *tag_bytes)
{
    coredata.tagsize_next = *tag_bytes;
    combuf->tagsize = *tag_bytes;
    *tag_bytes = coredata.tagsize;
}

void EXT_MEM_TEXT bsp_send(int pid, const void *tag, const void *payload, int nbytes)
{
    unsigned int index;
    unsigned int payload_offset;
    unsigned int total_nbytes = coredata.tagsize + nbytes;

    ebsp_message_queue* q = &combuf->message_queue[coredata.read_queue_index ^ 1];

    e_mutex_lock(0, 0, &coredata.payload_mutex);

    index = q->count;
    payload_offset = combuf->data_payloads.buffer_size;

    if ((payload_offset + total_nbytes > MAX_PAYLOAD_SIZE)
            || (index >= MAX_MESSAGES)) {
        index = -1;
        payload_offset = -1;
    } else {
        q->count++;
        combuf->data_payloads.buffer_size += total_nbytes;
    }

    e_mutex_unlock(0, 0, &coredata.payload_mutex);

    if (index == -1)
        return ebsp_message(err_send_overflow);

    // We are now ready to save the request and payload
    void* tag_ptr = &combuf->data_payloads.buf[payload_offset];
    payload_offset += coredata.tagsize;
    void* payload_ptr = &combuf->data_payloads.buf[payload_offset];

    q->message[index].pid = pid;
    q->message[index].tag = tag_ptr;
    q->message[index].payload = payload_ptr;
    q->message[index].nbytes = nbytes;

    memcpy(tag_ptr, tag, coredata.tagsize);
    memcpy(payload_ptr, payload, nbytes);
}

// Gets the next message from the queue, does not pop
// Returns 0 if no message
ebsp_message_header* EXT_MEM_TEXT _next_queue_message()
{
    ebsp_message_queue* q = &combuf->message_queue[coredata.read_queue_index];
    int qsize = q->count;

    // currently searching at message_index
    for (; coredata.message_index < qsize; coredata.message_index++) {
        if (q->message[coredata.message_index].pid != coredata.pid)
            continue;
        return &q->message[coredata.message_index];
    }
    return 0;
}

void _pop_queue_message()
{
    coredata.message_index++;
}

void EXT_MEM_TEXT bsp_qsize(int *packets, int *accum_bytes)
{
    *packets = 0;
    *accum_bytes = 0;

    ebsp_message_queue* q = &combuf->message_queue[coredata.read_queue_index];
    int mindex = coredata.message_index;
    int qsize = q->count;

    // currently searching at message_index
    for (; mindex < qsize; mindex++) {
        if (q->message[mindex].pid != coredata.pid)
            continue;
        *packets += 1;
        *accum_bytes += q->message[mindex].nbytes;
    }
    return;
}

void EXT_MEM_TEXT bsp_get_tag(int *status, void *tag)
{
    ebsp_message_header* m = _next_queue_message();
    if (m == 0) {
        *status = -1;
        return;
    }
    *status = m->nbytes;
    memcpy(tag, m->tag, coredata.tagsize);
}

void EXT_MEM_TEXT bsp_move(void *payload, int buffer_size)
{
    ebsp_message_header* m = _next_queue_message();
    _pop_queue_message();
    if (m == 0)  // This part is not defined by the BSP standard
        return;

    if (buffer_size == 0)  // Specified by BSP standard
        return;

    if (m->nbytes < buffer_size)
        buffer_size = m->nbytes;

    memcpy(payload, m->payload, buffer_size);
}

int EXT_MEM_TEXT bsp_hpmove(void **tag_ptr_buf, void **payload_ptr_buf)
{
    ebsp_message_header* m = _next_queue_message();
    _pop_queue_message();

    if (m == 0)
        return -1;

    *tag_ptr_buf = m->tag;
    *payload_ptr_buf = m->payload;
    return m->nbytes;
}

void EXT_MEM_TEXT ebsp_send_up(const void *tag, const void *payload, int nbytes)
{
    coredata.read_queue_index = 1;
    return bsp_send(-1, tag, payload, nbytes);
}
