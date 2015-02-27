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
#include <stdarg.h> //for va_list
#include <string.h>

//All bsp variables for this core
ebsp_core_data coredata;
ebsp_comm_buf* comm_buf = (ebsp_comm_buf*)COMMBUF_EADDR;

void _write_syncstate(int state);

void bsp_begin()
{
    int i;
    int row = e_group_config.core_row;
    int col = e_group_config.core_col;
    int cols = e_group_config.group_cols;

    // Initialize local data
    coredata.pid = col + cols*row;
    coredata.msgflag = 0;
    for(i = 0; i < MAX_N_REGISTER*coredata.nprocs; i++)
        coredata.registermap[i] = 0;

    // Send &coredata to ARM so that ARM can fill it with values
    comm_buf->coredata[coredata.pid] = &coredata;
    // Wait untill ARM received coredata so we can start
    // Accomplish this with a bsp_sync but using a different flag
    _write_syncstate(STATE_INIT);
    while (coredata.syncstate != STATE_CONTINUE) {}
    _write_syncstate(STATE_RUN);

    // Now the ARM has entered nprocs and remotetimer

    // Initialize epiphany timer
    e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);//TODO: E_CTIMER_CLK IS ONLY 255?
    coredata.initial_time = e_ctimer_get(E_CTIMER_0);
}

void bsp_end()
{
    bsp_sync();
    _write_syncstate(STATE_FINISH);
    //TODO: halt execution
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
    unsigned int _current_time = e_ctimer_get(E_CTIMER_0);
#ifdef DEBUG
    if(_current_time == 0)
        return -1.0;
#endif
    return (coredata.initial_time - _current_time)/CLOCKSPEED;
}

float bsp_remote_time()
{
    return coredata.remotetimer;
}

// Sync
void bsp_sync()
{
    _write_syncstate(STATE_SYNC);
    while (coredata.syncstate != STATE_CONTINUE) {}
	_write_syncstate(STATE_RUN);
}

void _write_syncstate(int state)
{
    coredata.syncstate = state; //local variable
    comm_buf->syncstate[coredata.pid] = state; //being polled by ARM
}

// Memory
void bsp_push_reg(const void* variable, const int nbytes)
{
    comm_buf->pushregloc[coredata.pid] = (void*)variable;
}

inline int row_from_pid(int pid)
{
    return pid / e_group_config.group_cols;
}

inline int col_from_pid(int pid)
{
    return pid % e_group_config.group_cols;
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    int slotID;
    void* adj_dst;
    for(slotID=0; ; slotID++) {
#ifdef DEBUG
        if(slotID >= MAX_N_REGISTER)
        {
            ebsp_message("ERROR: bsp_hpput(%d, %p, %p, %d, %d) could not find dst", pid, src, dst, offset, nbytes);
            return;
        }
#endif
        //Find the slot for our local _pid
        if(coredata.registermap[coredata.nprocs*slotID + coredata.pid] == dst)
        {
            //Then get the entry of remote pid
            adj_dst = coredata.registermap[coredata.nprocs*slotID + pid];
            break;
        }
    }
    adj_dst = (void*)((int)adj_dst + offset);
    e_write(&e_group_config, src,
            row_from_pid(pid),
            col_from_pid(pid),
            adj_dst, nbytes);
}

void ebsp_message(const char* format, ... )
{
    //TODO: Fix
    return;

    // First construct the message locally
    ebsp_message_buf b;
    va_list args;
    va_start(args, format);
    vsnprintf(&b.msg[0], sizeof(b.msg), format, args);
    va_end(args);

    // Check if ARM core has written the previous message
    // so that we can overwrite the previous buffer
    while (coredata.msgflag != 0) {}
    coredata.msgflag = 1;

    // First write the message
    memcpy(&comm_buf->message[coredata.pid], &b, sizeof(ebsp_message_buf));
    // Then write the flag indicating that the message is complete
    comm_buf->msgflag[coredata.pid] = 1;
}

