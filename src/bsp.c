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

#include "bsp.h"
#include <e-lib.h>

#define _NPROCS 16

int _nprocs = -1;
int _pid = -1;
e_barrier_t sync_bar[_NPROCS];
e_barrier_t* sync_bar_tgt[_NPROCS];

void bsp_begin()
{
    int row = e_group_config.core_row;
    int col = e_group_config.core_col;
    int cols = e_group_config.group_cols;
    _pid = col + cols*row;

    int* nprocs_loc = (int*)0x7500;
    _nprocs = (*nprocs_loc);

    e_barrier_init(sync_bar, sync_bar_tgt);
}

void bsp_end()
{
    // Nothing to do yet.
    return;
}

int bsp_nprocs()
{
    return _nprocs;
}


int bsp_pid()
{
    return _pid;
}

// Sync
void bsp_sync()
{
    // wait until each core is in this function:
    // e_barrier(sync_bar, sync_bar_tgt);

    // do memory management here, and halt when asking for RAM

    // wait again until everything sent
    e_barrier(sync_bar, sync_bar_tgt);

    // now ready to receive everything and continue

    return;
}

// Memory
void bsp_push_reg(const void* variable, const int nbytes)
{
    void* offset=0;//TODO shoud write to void** registermap_buffer 
    int voidSize=8;
    int intSize=8;	
    // @Abe: dit compileert niet
    //e_write(e_emem_config, **variable, 0u, 0u, offset+(voidSize+intSize)*e_core_id(), 8);
    //e_write(e_emem_config, *nbytes, 0u, 0u, offset+voidSize+(voidSize+intSize)*e_core_id(), 8);
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    unsigned int row, col;
    // @Abe: laten we hiervoor gebruik maken van een equivalent als _get_p_coords()
    // want deze shit werkt maar rare core ids..
    //e_coords_from_coreid(pid, &row, &col)

    int slotID;
    for(slotID=0; ; slotID++) {
#ifdef DEBUG
        if(slotID >= MAX_N_REGISTER) {
            fprintf(stderr,"PUTTING TO UNREGISTERED VARIABLE");
        }
#endif
        // @Abe: dit compileert niet
        //if(registermap[slotID][e_core_id()] == dst)
        //break;
    }

    void* actualDst=registermap[slotID][pid]+offset;

    // @Abe: dit compileert niet
    //e_write(e_group_config, src, row, col, actualDst, nbytes);	
}
