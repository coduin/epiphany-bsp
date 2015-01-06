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

#pragma once

#include "common.h"

//const char registermap_buffer_shm_name[] = REGISTERMAP_BUFFER_SHM_NAME;

/** Starts the BSP program.
 */
void bsp_begin();

/** Finalizes and cleans up the BSP program.
 */
void bsp_end();

/** Returns the number of available processors.
 *
 *  @return nprocs: An integer indicating the number of available processors.
 */
int bsp_nprocs();

/** Returns the processor ID of the local core
 *
 *  @return pid: An integer indicating the id of the local core
 */
int bsp_pid();

/** Time in seconds that has passed since bsp_begin() was called
 *
 *  @return t: A floating point value indicating the passed time in seconds.
 */
double bsp_time();

/** Terminates a superstep, and starts all communication. The computation is 
 *  halted until all communication has been performed.
 *  Somehow host_sync() is called before all processes continue.
 */
void bsp_sync();

/** Variable with value STATE_RUN, STATE_SYNC or STATE_CONTINUE
 *  This is needed to allow synchronisation on the ARM.
 */
int* syncstate;

//void*** registermap;//registermap[slotID][pid]=void*
void** registermap;//registermap[nprocs*slotID+pid]=void*

/** Registers a variable by putting to static memory location in host memory
 *  Registration maps are updated at next sync.
 */
void bsp_push_reg(const void* variable, const int nbytes);

/** Puts a variable to some processor.
 *  Internal workings:
 *  Loop over void*[nRegisteredVariables][sourcePid] to find variable
 *  Then use epiphany put to void*[index][targetPid]
 */
void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes);
