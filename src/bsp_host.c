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
#include <e-hal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct _bsp_state_t
{
    // The number of processors available
    int n_procs;

    // Maintain stack for every processor?
    int* memory;

    // The name of the e-program
    char* e_name;

    // Epiphany specific variables
    e_platform_t platform;
    e_epiphany_t dev;
} bsp_state_t;

// Global state
bsp_state_t state;

int bsp_init(const char* _e_name,
        int argc,
        char **argv)
{
    // Initialize the Epiphany system for the working with the host application
    if(e_init(0) != E_OK) {
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
    state.n_procs = state.platform.rows * state.platform.cols;

    // Copy the name to the state
    strcpy(state.e_name, _e_name);

    return 1;
}

int spmd_epiphany()
{
    // Start the program
    e_start_group(&state.dev);
    
    while(1) {
        // tmp solution, assume finishes in one seconds
        usleep(1000000);
        break;
    }

    return 1;
}

int bsp_begin(int nprocs)
{
    // Open the workgroup
    if(e_open(&state.dev,
            state.platform.row,
            state.platform.col,
            nprocs / state.platform.rows,
            nprocs / (nprocs / state.platform.rows) != E_OK))
    {
        fprintf(stderr, "ERROR: Could not open workgroup.\n");
        return 0;
    }

    printf("(BSP) INFO: Made a workgroup of size %i\n",
            nprocs / (nprocs / state.platform.rows) * (nprocs / state.platform.rows));

    // Load the e-binary
    if(e_load_group(state.e_name,
                state.dev,
                0, 0,
                state.dev.rows, state.dev.cols, // TODO: hard coded, should be fixed
                E_FALSE) != E_OK)
    {
        fprintf(stderr, "ERROR: Could not load program in workgroup.\n");
        return 0;
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
    return state.n_procs;
}
