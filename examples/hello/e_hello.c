/*
File: e_hello.c

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

#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    char* a = (void*)0x6000;
    (*a) = hmsg[p];
    int* po = (int*)0x6004;
    (*po) = p;

    ebsp_message("I am core %d and I will write '%c' to %p", p, *a, a);

    bsp_end();

    return 0;
}
