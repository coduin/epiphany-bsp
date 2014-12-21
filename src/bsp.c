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

int _nprocs = -1;
int _pid = -1;

void bsp_begin()
{
    int row, col;
    e_coreid_t cid = e_get_coreid();
    e_coords_from_coreid(cid, &row, &col);
    _pid = CORE_ID;

    int* nprocs_loc = (int*)0x7000;
    _nprocs = (*nprocs_loc);

    registermap=(void*)REGISTERMAP_ADRESS;
    //Set memory to 0 (dirty solution) TODO make clean solution
    int i;
    for(i = 0; i < MAX_N_REGISTER*_nprocs; i++) {
        void *tmp=registermap+i;
        *tmp=0;
    }
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
    // call mem_sync() global
    //
    // start barrier
    return;
}

// Memory
void bsp_push_reg(const void* variable, const int nbytes)
{
    void* offset=REGISTERMAP_BUFFER_ADRESS;
    e_write(&e_emem_config, &variable, 0u, 0u, offset+sizeof(void*)*CORE_ID, 8);
    e_write(&e_emem_config, &nbytes, 0u, 0u, offset+voidSize+sizeof(void*)*CORE_ID, 8);
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    int slotID;
    for(slotID=0; ; slotID++) {
#ifdef DEBUG
        if(slotID >= MAX_N_REGISTER) {
            //fprintf(stderr,"PUTTING TO UNREGISTERED VARIABLE");//THIS COMMAND DOES NOT WORK
        }
#endif
        if(registermap[_nprocs*slotID+CORE_ID] == dst)
            break;
    }

    void* actualDst=registermap[_nprocs*slotID+pid]+offset;
    e_write(&e_group_config, src, CORE_ROW, CORE_COL, actualDst, nbytes);	
}
