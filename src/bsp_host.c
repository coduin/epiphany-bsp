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

typedef struct _bsp_state_t
{
    int n_procs;

    // Maintain stack for every processor?
    int* memory;

    // epiphany specific variables
    e_platform_t platform;
} bsp_state_t;

// Global state
bsp_state_t state;

int bsp_init(const char* e_name,
        int argc,
        char **argv)
{
    // Initialize the Epiphany system for the working with the host application
    if(e_init(0) != E_OK) {
        fprintf(stderr, "ERROR: Could not initialize HAL data structures.");
        return 0;
    }

    // Get information on the platform
    if(!e_get_platform_info(&state.platform)) {
        fprintf(stderr, "ERROR: Could not obtain platform information.");
    }

    // Obtain the number of processors from the platform informatino
    state.n_procs = state.platform.num_chips;

    return 1;
}

int bsp_begin(int nprocs)
{
    return 1;
}

int bsp_end()
{
    if(e_finalize() != E_OK) {
         fprintf(stderr, "ERROR: Could not finalize the Epiphany connection.");
        return 0;
    }
    return 1;
}

int bsp_nprocs()
{
    return state.n_procs;
}
