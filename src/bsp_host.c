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

typedef struct _bsp_state_t
{
    int n_procs;

    // Maintain stack for every processor?
    int* memory;
} bsp_state_t;

// Global state
bsp_state_t state;

void bsp_init(const char* e_name,
        int argc,
        char **argv)
{
    state.n_procs = 0;
}

void bsp_begin(int nprocs)
{
    return;
}

void bsp_end()
{
    return;
}

int bsp_nprocs()
{
    return state.n_procs;
}
