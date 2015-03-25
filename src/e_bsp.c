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

#include "e_bsp.h"
#include <e-lib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// All bsp variables for this core
ebsp_core_data coredata;
ebsp_comm_buf* comm_buf = (ebsp_comm_buf*)COMMBUF_EADDR;

// The following variables belong in ebsp_core_data but since
// we do not want to include e-lib.h in common.h (as it is used by host)
// we place these variables here

// FIXME: sync_barrier and payload_mutex, are assumed
// to have the exact same address on all cores.
// Currently this is always the case
volatile e_barrier_t    sync_barrier[_NPROCS];
e_barrier_t*            sync_barrier_tgt[_NPROCS];
e_mutex_t               payload_mutex;

// All error messages are written here so that we can store
// them in external ram if needed
const char err_pushreg_multiple[] = "BSP ERROR: multiple bsp_push_reg calls within one sync";
const char err_pushreg_overflow[] = "BSP ERROR: Trying to push more than MAX_BSP_VARS vars";
const char err_var_not_found[]    = "BSP ERROR: could not find bsp var. targetpid %d, addr = %p";
const char err_get_overflow[]     = "BSP ERROR: too many bsp_get requests per sync";
const char err_put_overflow[]     = "BSP ERROR: too many bsp_put requests per sync";
const char err_put_overflow2[]    = "BSP ERROR: too large bsp_put payload per sync";

void _write_syncstate(int state);
int row_from_pid(int pid);
int col_from_pid(int pid);

void bsp_begin()
{
    int i, j;
    int row = e_group_config.core_row;
    int col = e_group_config.core_col;
    int cols = e_group_config.group_cols;

    // Initialize local data
    coredata.pid = col + cols * row;
    coredata.msgflag = 0;
    coredata.request_counter = 0;
    coredata.var_pushed = 0;

    // Initialize the barrier used during syncs
    e_barrier_init(sync_barrier, sync_barrier_tgt);

    // Initialize the mutex for bsp_put
    e_mutex_init(0, 0, &payload_mutex, MUTEXATTR_NULL);

    // Send &coredata to ARM so that ARM can fill it with values
    comm_buf->coredata[coredata.pid] = &coredata;
    // Wait untill ARM received coredata so we can start
    // Accomplish this with a bsp_sync but using a different flag
    _write_syncstate(STATE_INIT);
    while (coredata.syncstate != STATE_CONTINUE) {}
    _write_syncstate(STATE_RUN);

    // Now the ARM has entered nprocs

    // Initialize epiphany timer
    coredata.time_passed = 0.0f;
    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
    coredata.last_timer_value = e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
}

void bsp_end()
{
    bsp_sync();
    _write_syncstate(STATE_FINISH);
    // TODO: halt execution
}

int bsp_nprocs()
{
    return coredata.nprocs;
}


int bsp_pid()
{
    return coredata.pid;
}

float bsp_time()
{
    // TODO: Add timer overhead the calculation
    unsigned int cur_time = e_ctimer_get(E_CTIMER_0);
    coredata.time_passed += (coredata.last_timer_value - cur_time) / CLOCKSPEED;
    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
    // Tested: between setting E_CTIMER_MAX and 
    // reading the timer, it decreased by 23 clockcycles
    coredata.last_timer_value = e_ctimer_get(E_CTIMER_0);
    //coredata.last_timer_value = cur_time;

#ifdef DEBUG
    if (cur_time == 0)
        return -1.0f;
#endif
    return coredata.time_passed;
}

float bsp_remote_time()
{
    return comm_buf->remotetimer;
}

// Sync
void bsp_sync()
{
    int i;
    ebsp_data_request* reqs = &comm_buf->data_requests[coredata.pid][0];

    // First handle all bsp_get requests
    // Then handle all bsp_put requests (because of bsp specifications)
    // They are stored in the same list and recognized by the
    // highest bit of nbytes

    e_barrier(sync_barrier, sync_barrier_tgt);
    for (i = 0; i < coredata.request_counter; ++i)
    {
        // Check if this is a get
        if ((reqs[i].nbytes & DATA_PUT_BIT) == 0)
            memcpy(reqs[i].dst,
                    reqs[i].src,
                    reqs[i].nbytes & ~DATA_PUT_BIT);
    }
    e_barrier(sync_barrier, sync_barrier_tgt);
    for (i = 0; i < coredata.request_counter; ++i)
    {
        // Check if this is a put
        if ((reqs[i].nbytes & DATA_PUT_BIT) != 0)
            memcpy(reqs[i].dst,
                    reqs[i].src,
                    reqs[i].nbytes & ~DATA_PUT_BIT);
    }
    coredata.request_counter = 0;

    // This can be done at any point during the sync
    // (as long as it is after the first barrier so all cores are syncing)
    // and only one core needs to set this, but this also works
    comm_buf->data_payloads.buffer_size = 0;

    if (coredata.var_pushed)
    {
        coredata.var_pushed = 0;
        if (coredata.pid == 0)
            comm_buf->bsp_var_counter++;
    }

    // Synchronize with host
    //_write_syncstate(STATE_SYNC);
    //while (coredata.syncstate != STATE_CONTINUE) {}
    //_write_syncstate(STATE_RUN);
    e_barrier(sync_barrier, sync_barrier_tgt);
}

void _write_syncstate(int state)
{
    coredata.syncstate = state; // local variable
    comm_buf->syncstate[coredata.pid] = state; // being polled by ARM
}

void bsp_push_reg(const void* variable, const int nbytes)
{
    if (coredata.var_pushed)
        return ebsp_message(err_pushreg_multiple);

    if (comm_buf->bsp_var_counter == MAX_BSP_VARS)
        return ebsp_message(err_pushreg_overflow);

    comm_buf->bsp_var_list[comm_buf->bsp_var_counter][coredata.pid] =
        (void*)variable;

    coredata.var_pushed = 1;
}

int row_from_pid(int pid)
{
    return pid / e_group_config.group_cols;
}

int col_from_pid(int pid)
{
    return pid % e_group_config.group_cols;
}

// This incoroporates the bsp_var_list as well as
// the epiphany global address system
// The resulting address can be written to directly
void* _get_remote_addr(int pid, const void *addr, int offset)
{
    // Find the slot for our local pid
    // And return the entry for the remote pid including the epiphany mapping
    int slot;
    for(slot = 0; slot < MAX_BSP_VARS; ++slot)
        if (comm_buf->bsp_var_list[slot][coredata.pid] == addr)
            return e_get_global_address(row_from_pid(pid),
                    col_from_pid(pid),
                    (void*)((int)comm_buf->bsp_var_list[slot][pid] + offset));
    ebsp_message(err_var_not_found, pid, addr);
    return 0;
}

void bsp_put(int pid, const void *src, void *dst, int offset, int nbytes)
{
    // Check if we can store the request
    if (coredata.request_counter >= MAX_DATA_REQUESTS)
        return ebsp_message(err_put_overflow);

    // Find remote address
    void* dst_remote = _get_remote_addr(pid, dst, offset);
    if (!dst_remote) return;

    // Check if we can store the payload
    // A mutex is needed for this.
    // While holding the mutex this core checks if it can store
    // the payload and if so, updates the buffer
    // Note that the mutex is NOT held while writing the payload itself
    // A possible error message is given after unlocking
    unsigned int payload_offset;

    e_mutex_lock(0, 0, &payload_mutex);

    payload_offset = comm_buf->data_payloads.buffer_size;

    if (payload_offset + nbytes > MAX_PAYLOAD_SIZE)
        payload_offset = -1;
    else
        comm_buf->data_payloads.buffer_size += nbytes;

    e_mutex_unlock(0, 0, &payload_mutex);

    if (payload_offset == -1)
        return ebsp_message(err_put_overflow2);

    // We are now ready to save the request and payload
    void* payload_ptr = &comm_buf->data_payloads.buf[payload_offset];

    // TODO: Measure if e_dma_copy is faster here for both request and payload

    // Save request
    ebsp_data_request* req = &comm_buf->data_requests[coredata.pid][coredata.request_counter];
    req->src = payload_ptr;
    req->dst = dst_remote;
    req->nbytes = nbytes | DATA_PUT_BIT;
    coredata.request_counter++;

    // Save payload
    memcpy(payload_ptr, src, nbytes);
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    void* dst_remote = _get_remote_addr(pid, dst, offset);
    if (!dst_remote) return;
    memcpy(dst_remote, src, nbytes);
    //e_write(&e_group_config, src,
    //        row_from_pid(pid),
    //        col_from_pid(pid),
    //        adj_dst, nbytes);
}

void bsp_get(int pid, const void *src, int offset, void *dst, int nbytes)
{
    if (coredata.request_counter >= MAX_DATA_REQUESTS)
        return ebsp_message(err_get_overflow);
    const void* src_remote = _get_remote_addr(pid, src, offset);
    if (!src_remote) return;

    ebsp_data_request* req = &comm_buf->data_requests[coredata.pid][coredata.request_counter];
    req->src = src_remote;
    req->dst = dst;
    req->nbytes = nbytes;
    coredata.request_counter++;
}

void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes)
{
    const void* src_remote = _get_remote_addr(pid, src, offset);
    if (!src_remote) return;
    memcpy(dst, src_remote, nbytes);
    //e_read(&e_group_config, dst,
    //        row_from_pid(pid),
    //        col_from_pid(pid),
    //        adj_src, nbytes);
}

void ebsp_message(const char* format, ... )
{
    // Write the message to a buffer
    ebsp_message_buf b;
    va_list args;
    va_start(args, format);
    vsnprintf(&b.msg[0], sizeof(b.msg), format, args);
    va_end(args);

    // Check if ARM core has written the previous message
    // so that we can overwrite the previous buffer
    while (coredata.msgflag != 0) {}
    coredata.msgflag = 1;

    // Write the message to the external memory
    memcpy(&comm_buf->message[coredata.pid], &b, sizeof(ebsp_message_buf));
    // Write the flag indicating that the message is complete
    comm_buf->msgflag[coredata.pid] = 1;
}

