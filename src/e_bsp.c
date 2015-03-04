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

// The following belong in ebsp_core_data but since
// we do not want to include e-lib.h in common.h (as it is used by host)
// we place these variables here
// FIXME: the first of these two, sync_barrier, is assumed
// to have the exact same address on all cores.
// Currently this is always the case
volatile e_barrier_t sync_barrier[_NPROCS];
e_barrier_t*         sync_barrier_tgt[_NPROCS];

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
    for(i = 0; i < MAX_N_REGISTER; ++i)
        for(j = 0; j < _NPROCS; ++j)
            coredata.registermap[i][j] = 0;
    coredata.get_counter = 0;
    coredata.put_counter = 0;

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
    e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
    coredata.last_timer_value = e_ctimer_get(E_CTIMER_0);

    // Initialize the barrier used during syncs
    e_barrier_init(sync_barrier, sync_barrier_tgt);
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
    unsigned int cur_time = e_ctimer_get(E_CTIMER_0);
    coredata.time_passed += (coredata.last_timer_value - cur_time) / CLOCKSPEED;
    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
    // Tested: between setting E_CTIMER_MAX and 
    // reading the timer, it decreased by 23 clockcycles
    coredata.last_timer_value = e_ctimer_get(E_CTIMER_0);
#ifdef DEBUG
    if (cur_time == 0)
        return -1.0;
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

    // Handle all bsp_get requests
    e_barrier(sync_barrier, sync_barrier_tgt);
    for (i = 0; i < coredata.get_counter; ++i)
    {
        ebsp_get_request* req = &comm_buf->get_requests[coredata.pid][i];
        memcpy(req->dst, req->src, req->nbytes);
    }

    // Handle all bsp_put requests
    e_barrier(sync_barrier, sync_barrier_tgt);
    for (i = 0; i < coredata.put_counter; ++i)
    {
        // TODO
    }

    // Synchronize with host
    _write_syncstate(STATE_SYNC);
    while (coredata.syncstate != STATE_CONTINUE) {}
	_write_syncstate(STATE_RUN);
}

void _write_syncstate(int state)
{
    coredata.syncstate = state; // local variable
    comm_buf->syncstate[coredata.pid] = state; // being polled by ARM
}

// Memory
void bsp_push_reg(const void* variable, const int nbytes)
{
    comm_buf->pushregloc[coredata.pid] = (void*)variable;
}

int row_from_pid(int pid)
{
    return pid / e_group_config.group_cols;
}

int col_from_pid(int pid)
{
    return pid % e_group_config.group_cols;
}

void* _get_remote_addr(int pid, const void *addr)
{
    // Find the slot for our local pid
    // And return the entry for the remote pid
    int slot;
    for(slot = 0; slot < MAX_N_REGISTER; ++slot)
        if (coredata.registermap[slot][coredata.pid] == addr)
            return coredata.registermap[slot][pid];
#ifdef DEBUG
    ebsp_message("BSP ERROR: could not find register. targetpid %d, addr = %p",
            pid, addr);
#endif
    return 0;
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    void* adj_dst = _get_remote_addr(pid, dst);
    if (!adj_dst) return;

    adj_dst = (void*)((int)adj_dst + offset);
    e_write(&e_group_config, src,
            row_from_pid(pid),
            col_from_pid(pid),
            adj_dst, nbytes);
}

void bsp_get(int pid, const void *src, int offset, void *dst, int nbytes)
{
#ifdef DEBUG
    if (coredata.get_counter >= MAX_GET_REQUESTS)
        return ebsp_message("BSP ERROR: too many bsp_get calls per sync");
#endif
    const void* adj_src = _get_remote_addr(pid, src);
    if (!adj_src) return;
    adj_src = (void*)((int)adj_src + offset);

    ebsp_get_request* req = &comm_buf->get_requests[coredata.pid][coredata.get_counter];
    req->src = e_get_global_address(row_from_pid(pid), col_from_pid(pid), adj_src);
    req->dst = dst;
    req->nbytes = nbytes;
    coredata.get_counter++;
}

void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes)
{
    const void* adj_src = _get_remote_addr(pid, src);
    if (!adj_src) return;

    adj_src = (const void*)((int)adj_src + offset);
    e_read(&e_group_config, dst,
            row_from_pid(pid),
            col_from_pid(pid),
            adj_src, nbytes);
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

