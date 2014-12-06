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

void bsp_begin(int nprocs)
{
    e_coreid_t cid = e_get_coreid();
    _pid = (int)cid;

    int* nprocs_loc = (int*)0x100;
    _nprocs = (*nprocs_loc);
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
