/*
File: e_dot_product.c

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

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    int base = 0x4000;
    int chunk = (*(int*)(base));
    int i = 0;
    int sum = 0;
    for(i = 0; i < chunk; ++i) {
        sum += (*(int*)(base + 4 + 4*i)) * (*(int*)(base + 4 + 4*(chunk + i)));
    }

    int* loc = (int*)(base + 4 + 32*8);
    (*loc) = sum;

    bsp_end();

    return 0;
}
