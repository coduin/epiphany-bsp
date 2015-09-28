/*
File: e_bsp.c

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
#include <stdio.h>
#include <stdarg.h>

ebsp_core_data coredata;

void _write_syncstate(int8_t state);

void EXT_MEM_TEXT bsp_begin()
{
    int row = e_group_config.core_row;
    int col = e_group_config.core_col;
    int cols = e_group_config.group_cols;

    // Initialize local data
    coredata.pid = col + cols * row;
    coredata.nprocs = combuf->nprocs;
    coredata.request_counter = 0;
    coredata.var_pushed = 0;
    coredata.tagsize = combuf->tagsize;
    coredata.tagsize_next = coredata.tagsize;
    coredata.read_queue_index = 0;
    coredata.message_index = 0;
    coredata.last_dma_desc = NULL;

    // Initialize the barrier and mutexes
    e_barrier_init(coredata.sync_barrier, coredata.sync_barrier_tgt);
    e_mutex_init(0, 0, &coredata.payload_mutex, MUTEXATTR_NULL);
    e_mutex_init(0, 0, &coredata.ebsp_message_mutex, MUTEXATTR_NULL);
    e_mutex_init(0, 0, &coredata.malloc_mutex, MUTEXATTR_NULL);

    // Barrier fix:
    // if core i is at ebsp_barrier but core j has not even done bsp_begin yet
    // then behaviour was undefined. The following line should fix this
    // by setting core0.sync_barrier[i] = 0
    *(coredata.sync_barrier_tgt[0]) = 0;

    _init_local_malloc();

    // Copy stream descriptors to local memory
    unsigned int nbytes = combuf->n_streams[coredata.pid] * sizeof(ebsp_stream_descriptor);
    coredata.local_streams = ebsp_malloc(nbytes);
    ebsp_aligned_transfer(coredata.local_streams, combuf->extmem_streams[coredata.pid], nbytes);

    // Send &syncstate to ARM
    if (coredata.pid == 0)
        combuf->syncstate_ptr = (int8_t*)&coredata.syncstate;

#ifdef DEBUG
    // Wait for ARM before starting
    _write_syncstate(STATE_EREADY);
    while (coredata.syncstate != STATE_CONTINUE) {}
#endif
    _write_syncstate(STATE_RUN);

    ebsp_barrier();

    // Initialize epiphany timer
    coredata.time_passed = 0.0f;
    ebsp_raw_time();
}

void bsp_end()
{
    _write_syncstate(STATE_FINISH);
    // Finish execution
    __asm__("trap 3");
}

int bsp_nprocs()
{
    return coredata.nprocs;
}

int bsp_pid()
{
    return coredata.pid;
}

float EXT_MEM_TEXT bsp_time()
{
    coredata.time_passed += ebsp_raw_time() / CLOCKSPEED;
    return coredata.time_passed;
}

float ebsp_host_time()
{
    return combuf->remotetimer;
}

// Sync
void bsp_sync()
{
    // Handle all bsp_get requests before bsp_put request. They are stored in
    // the same list and recognized by the highest bit of nbytes

    // Instead of copying the code twice, we put it in a loop
    // so that the code is shorter (this is tested)
    ebsp_data_request* reqs = &combuf->data_requests[coredata.pid][0];
    for (int put = 0;;)
    {
        e_barrier(coredata.sync_barrier, coredata.sync_barrier_tgt);
        for (int i = 0; i < coredata.request_counter; ++i)
        {
            int nbytes = reqs[i].nbytes;
            // Check if this is a get or a put
            if ((nbytes & DATA_PUT_BIT) == put)
                memcpy(reqs[i].dst, reqs[i].src, nbytes & ~DATA_PUT_BIT);
        }
        if (put == 0)
            put = DATA_PUT_BIT;
        else
            break;
    }
    coredata.request_counter = 0;

    // This can be done at any point during the sync
    // (as long as it is after the first barrier and before the last one
    // so all cores are syncing) and only one core needs to set this, but
    // letting all cores set it produces smaller code (binary size)
    combuf->data_payloads.buffer_size = 0;
    combuf->message_queue[coredata.read_queue_index].count = 0;
    // Switch queue between 0 and 1
    // xor seems to produce the shortest assembly
    coredata.read_queue_index ^= 1;

    if (coredata.var_pushed)
    {
        coredata.var_pushed = 0;
        if (coredata.pid == 0)
            combuf->bsp_var_counter++;
    }

    coredata.tagsize = coredata.tagsize_next;
    coredata.message_index = 0;

    e_barrier(coredata.sync_barrier, coredata.sync_barrier_tgt);
}

void ebsp_barrier()
{
    e_barrier(coredata.sync_barrier, coredata.sync_barrier_tgt);
}

void ebsp_host_sync()
{
    _write_syncstate(STATE_SYNC);
    while (coredata.syncstate != STATE_CONTINUE) {}
    _write_syncstate(STATE_RUN);
}

void _write_syncstate(int8_t state)
{
    coredata.syncstate = state;  // local variable
    combuf->syncstate[coredata.pid] = state;  // being polled by ARM
}

void EXT_MEM_TEXT bsp_abort(const char * format, ...)
{
    // Because of the way these arguments work we can not
    // simply call ebsp_message here
    // so this function contains a copy of ebsp_message

    // Write the message to a buffer
    char buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(&buf[0], sizeof(buf), format, args);
    va_end(args);

    // Lock mutex
    e_mutex_lock(0, 0, &coredata.ebsp_message_mutex);
    // Write the message
    memcpy(&combuf->msgbuf[0], &buf[0], sizeof(buf));
    combuf->msgflag = coredata.pid+1;
    // Wait for it to be printed
    while (combuf->msgflag != 0){}
    // Unlock mutex
    e_mutex_unlock(0, 0, &coredata.ebsp_message_mutex);

    // Abort all cores and notify host
    _write_syncstate(STATE_ABORT);
    // Experimental Epiphany feature that sends
    // and abort signal to all cores
    __asm__("MBKPT");
    // Halt this core
    __asm__("trap 3");
}

void EXT_MEM_TEXT ebsp_send_string(const char* string)
{
    // Lock mutex
    e_mutex_lock(0, 0, &coredata.ebsp_message_mutex);
    // Write the message
    ebsp_aligned_transfer(&combuf->msgbuf[0], string, sizeof(combuf->msgbuf));

    // Wait for message to be written
    _write_syncstate(STATE_MESSAGE);
    while (coredata.syncstate != STATE_CONTINUE) {};
    _write_syncstate(STATE_RUN);

    // Unlock mutex
    e_mutex_unlock(0, 0, &coredata.ebsp_message_mutex);
}

void EXT_MEM_TEXT ebsp_message(const char* format, ... )
{
    // Write the message to a buffer
    char buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(&buf[0], sizeof(buf), format, args);
    va_end(args);

    ebsp_send_string(buf);
}

