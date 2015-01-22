/*
File: bsp.h

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
#include <string.h>


int _nprocs = -1;
int _pid = -1;
volatile e_barrier_t  sync_bar[_NPROCS]      SECTION("section_core");
         e_barrier_t* sync_bar_tgt[_NPROCS]  SECTION("section_core");
e_memseg_t emem;

unsigned int _initial_time;


void e_barrier_ultra(volatile e_barrier_t bar_array[], e_barrier_t *tgt_bar_array[])
{
    int corenum, numcores, i;
    numcores = e_group_config.group_rows * e_group_config.group_cols;
    corenum = e_group_config.core_row * e_group_config.group_cols + e_group_config.core_col;
    // Barrier as a Flip-Flop
    // Flip pass
    if (corenum == 0)
    {
        // set "my" slot
        bar_array[corenum] = 1;
        // poll on all slots
        for (i=1; i<numcores; i++)
            while (bar_array[i] == 0) {};
    } else {
        // set "my" remote slot
        *(tgt_bar_array[corenum]) = 1;
    }
    // Flop pass
    if (corenum == 0)
    {
        // clear all local slots
        for (i=0; i<numcores; i++)
            bar_array[i] = 0;
        // set remote slots
        for (i=1; i<numcores; i++)
            *(tgt_bar_array[i]) = 1;
    } else {
        // poll on "my" local slot
        while (bar_array[corenum] == 0) {};
        // clear "my" local slot
        bar_array[corenum] = 0;
    }
    return;
}

void e_barrier_mega(volatile e_barrier_t bar_array[], e_barrier_t *tgt_bar_array[])
{
    int corenum, numcores, i;
    numcores = e_group_config.group_rows * e_group_config.group_cols;
    corenum = e_group_config.core_row * e_group_config.group_cols + e_group_config.core_col;
    // Barrier as a Flip-Flop
    if (corenum == 0)
    {
        // Flip pass
        // set "my" slot
        bar_array[corenum] = 1;
        // poll on all slots
        for (i=1; i<numcores; i++)
            while (bar_array[i] == 0) {};
        // Flop pass
        // clear all local slots
        for (i=0; i<numcores; i++)
            bar_array[i] = 0;
        // set remote slots
        for (i=1; i<numcores; i++)
            *(tgt_bar_array[i]) = 1;
    } else {
        // Flip pass
        // set "my" remote slot
        *(tgt_bar_array[0]) = 1;
        // Flop pass
        // poll on "my" local slot
        while (bar_array[0] == 0) {};
        // clear "my" local slot
        bar_array[0] = 0;
    }
    return;
}



inline int row_from_pid(int pid)
{
    return pid / e_group_config.group_cols;
}

inline int col_from_pid(int pid)
{
    return pid % e_group_config.group_cols;
}

void bsp_begin()
{
    int i;

    int row = e_group_config.core_row;
    int col = e_group_config.core_col;
    int cols = e_group_config.group_cols;
    _pid = col + cols*row;

    int* nprocs_loc = (int*)NPROCS_LOC_ADDRESS;
    _nprocs = (*nprocs_loc);

    char rm_name[10];
    strcpy(rm_name, REGISTERMAP_BUFFER_SHM_NAME);
    char id[4] = { 0 };
    sprintf(id, "_%i", _pid);
    for(i = 6; i <= 9; i++) {
        rm_name[i] = id[i - 6];
    }
    e_shm_attach(&emem, rm_name);

    registermap = (void**)REGISTERMAP_ADDRESS;
	syncstate = (int*)SYNC_STATE_ADDRESS;
    (*syncstate) = STATE_RUN;

    //Set memory to 0 (dirty solution) TODO make clean solution
    for(i = 0; i < MAX_N_REGISTER*_nprocs; i++)
        registermap[i] = 0;

    e_barrier_init(sync_bar, sync_bar_tgt);

    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
    e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
    _initial_time = e_ctimer_get(E_CTIMER_0);
}

void bsp_end()
{
    bsp_sync();
	(*syncstate) = STATE_FINISH;
}

int bsp_nprocs()
{
    return _nprocs;
}


int bsp_pid()
{
    return _pid;
}

float bsp_time()
{
    unsigned int _current_time = e_ctimer_get(E_CTIMER_0);
#ifdef DEBUG
    if(_current_time == 0)
        return -1.0;
#endif
    return (_initial_time - _current_time)/CLOCKSPEED;
}

// Sync
void bsp_sync()
{
	//Signal host that epiphany is syncing, wait until host is done
	(*syncstate) = STATE_SYNC;

	while(*syncstate != STATE_CONTINUE) {
        e_wait(E_CTIMER_1, 10000);
    }
    e_barrier_init(sync_bar, sync_bar_tgt);
    e_barrier_mega(sync_bar, sync_bar_tgt);
	
	//Reset state
	(*syncstate) = STATE_RUN;
}

// Memory
void bsp_push_reg(const void* variable, const int nbytes)
{
    e_write((void*)&emem,
            &variable,
            0, 0, 0, 
            sizeof(void*));
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    int slotID;
    for(slotID=0; ; slotID++) {
#ifdef DEBUG
        if(slotID >= MAX_N_REGISTER) {
            break;
        }
#endif
        if(registermap[_nprocs*slotID+pid] == dst)
            break;
    }

    void* adj_dst = registermap[_nprocs * slotID + pid] + offset;
    e_write(&e_group_config, src,
            row_from_pid(pid),
            col_from_pid(pid),
            adj_dst, nbytes);	
}
