/*
File: bsp_host.c

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

#include "bsp_host.h"
#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// Global state
bsp_state_t state;

void _get_p_coords(int pid, int* row, int* col)
{
    (*row) = pid / state.cols;
    (*col) = pid % state.cols;
}

bsp_state_t* _get_state()
{
    return &state;
}

int bsp_init(const char* _e_name,
		int argc,
		char **argv)
{
    // Initialize the Epiphany system for the working with the host application
    if(e_init(NULL) != E_OK) {
        fprintf(stderr, "ERROR: Could not initialize HAL data structures.\n");
        return 0;
    }

    // Reset the Epiphany system
    if(e_reset_system() != E_OK) {
        fprintf(stderr, "ERROR: Could not reset the Epiphany system.\n");
        return 0;
    }

    // Get information on the platform
    if(e_get_platform_info(&state.platform) != E_OK) {
        fprintf(stderr, "ERROR: Could not obtain platform information.\n");
        return 0;
    }

    // Obtain the number of processors from the platform information
    state.nprocs = state.platform.rows * state.platform.cols;

    // Copy the name to the state
    state.e_name = (char*)malloc(MAX_NAME_SIZE);
    strcpy(state.e_name, _e_name);

    return 1;
}

int spmd_epiphany()
{
    // Start the program
    e_start_group(&state.dev);

    // sleep for 1.0 seconds
    usleep(100000); //10^6 microseconds

    // @abe: er zijn geen bools in C! voorlopig uitgecomment
	/* while(true) {
		//Listen for syncs
		//Epiphany should somehow wait for global sync..
		if(syncing) {
			host_sync();	
			syncing=false;
		}	
    } */

    printf("(BSP) INFO: Program finished\n");

    return 1;
}

int bsp_begin(int nprocs)
{
    state.rows = (nprocs / state.platform.rows);
    state.cols = nprocs / (nprocs / state.platform.rows);

    printf("(BSP) INFO: Making a workgroup of size %i x %i\n",
            state.rows,
            state.cols);

    state.nprocs_used = nprocs;

    // Open the workgroup
    if(e_open(&state.dev,
            0, 0,
            state.rows,
            state.cols) != E_OK)
    {
        fprintf(stderr, "ERROR: Could not open workgroup.\n");
        return 0;
    }
    
    if(e_reset_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: Could not reset workgroup.\n");
        return 0;
    }


    // Load the e-binary
    printf("(BSP) INFO: Loading: %s\n", state.e_name);
    if(e_load_group(state.e_name,
                &state.dev,
                0, 0,
                state.rows, state.cols, 
                E_FALSE) != E_OK)
    {
        fprintf(stderr, "ERROR: Could not load program in workgroup.\n");
        return 0;
    }

    // Write the nprocs to memory
    int i, j;
    for(i = 0; i < state.platform.rows; ++i) {
        for(j = 0; j < state.platform.cols; ++j) {
            e_write(&state.dev, i, j, (off_t)0x7500, &state.nprocs, sizeof(int));
        }
    }

    return 1;
}

int bsp_end()
{
	if(e_finalize() != E_OK) {
		fprintf(stderr, "ERROR: Could not finalize the Epiphany connection.\n");
		return 0;
	}
	return 1;
}

int bsp_nprocs()
{
	return state.nprocs;
}

void host_sync() {
	mem_sync();
}

//Memory
void mem_sync() {
	//Broadcast void** registermap_buffer to all void*** registermap
	//Then reset registermap_buffer
	
	//Right now bsp_pop_reg is ignored
	//Check if overwrite is necessary => this gives no problems
	
	//TODO: write this function
}

