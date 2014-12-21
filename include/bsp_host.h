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

#include <e-hal.h>
#include <e-loader.h>

typedef struct _bsp_state_t
{
    // The number of processors available
    int nprocs;

    // Maintain stack for every processor?
    int* memory;

    // The name of the e-program
    char* e_name;

    // Number of rows or columns in use
    int rows;
    int cols;

    // Number of processors in use
    int nprocs_used;

    // Epiphany specific variables
    e_platform_t platform;
    e_epiphany_t dev;
} bsp_state_t;

bsp_state_t* _get_state();

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


void** registermap_buffer;
int nVariablesRegistered;

/** host_sync is called on bsp_sync() in epiphany
 */
void host_sync();

/** The part of host_sync responsible for registering variables
 *  This broadcasts void** registermap_buffer to void*** registermap
 */
void mem_sync();
