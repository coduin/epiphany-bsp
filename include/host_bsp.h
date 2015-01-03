/*
File: bsp_host.h

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

#include <e-hal.h>
#include <e-loader.h>
#include "common.h"

//const char registermap_buffer_shm_name[] = REGISTERMAP_BUFFER_SHM_NAME;

typedef struct _bsp_state_t
{
    // The number of processors available
    int nprocs;

    // The name of the e-program
    char* e_name;

    // Number of rows or columns in use
    int rows;
    int cols;

    // Number of processors in use
    int nprocs_used;

    // Register map
    e_mem_t registermap_buffer;
    int num_vars_registered;

    // Epiphany specific variables
    e_platform_t platform;
    e_epiphany_t dev;
} bsp_state_t;

/** This writes data from the host processor to the co-processor.
 *  This can be useful for distributing initial data, or when dividing work
 *  betewen the host and the Epiphany.
 *
 *  @param pid: The processor ID in the BSP system.
 *  @param src: A pointer to the source of the data to write.
 *  @param dst: The destination on the chip processor. (e.g. 0x2000)
 *  @param size: Amount of data to write in bytes.
 */
void co_write(int pid, void* src, off_t dst, int size);

/** This reads data from the co-processor to the host processor.
 *  This can be useful for distributing initial data, or when dividing work
 *
 *  @param pid: The processor ID in the BSP system.
 *  @param src: The source of the data on the co-processor (e.g. 0x2000)
 *  @param dst: A destination pointer on the host processor.
 *  @param size: Amount of data to read in bytes.
 */
void co_read(int pid, off_t src, void* dst, int size);

/** Initializes the BSP system. This sets up all the BSP variables and loads
 *  the epiphany BSP program.
 *
 *  @param e_name: A string containing the name of the eBSP program.
 *  @param argc: An integer containing the number of input arguments
 *  @param argv: An array of strings containg the input flags.
 *  @return flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int bsp_init(const char* e_name,
		int argc,
		char **argv);

/** Starts the SPMD program on the Epiphany cores.
 *
 *  @return flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int spmd_epiphany();

/** Starts the BSP program.
 *
 *  @param nprocs: An integer indicating the number of processors to run on.
 *  @return flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int bsp_begin(int nprocs);

/** Finalizes and cleans up the BSP program.
 *  @return flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int bsp_end();

/** Returns the number of available processors.
 *
 *  @return nprocs: An integer indicating the number of available processors.
 */
int bsp_nprocs();

//------------------
// Private functions
//------------------

// For synchronization between host and co
void _host_sync();

// Convenience functions
void _get_p_coords(int pid, int* row, int* col)
bsp_state_t* _get_state();
