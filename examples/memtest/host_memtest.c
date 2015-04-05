/*
File: host_memtest.c

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

#include <host_bsp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // initialize the BSP system
    if(!bsp_init("bin/e_memtest.srec", argc, argv))
        printf("init failed\n");

    // initialize the epiphany system, and load the e-program
    if(!bsp_begin(bsp_nprocs()))
        printf("begin failed\n");

    // run the SPMD on the e-cores
    ebsp_spmd();

    // read messages
    printf("Reading results...\n");
    int packets;
    int accum_bytes;
    ebsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++)
    {
        int value;
        ebsp_move(&value, sizeof(int));
        printf("%i: %d\n", i, value);
    }

    // finalize
    bsp_end();

    return 0;
}
