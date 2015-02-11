/*
File: common.h

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
    if(!bsp_init("bin/e_hello.srec", argc, argv)) {
        fprintf(stderr, "[HELLO] bsp_init() failed\n");
        return -1;
    }

    // show the number of processors available
    printf("bsp_nprocs(): %i\n", bsp_nprocs());

    // initialize the epiphany system, and load the e-program
    if(!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[HELLO] bsp_begin() failed\n");
        return -1;
    }

    // run the SPMD on the e-cores
    ebsp_spmd();

    // read messages
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        int p;
        co_read(pid, (off_t)0x6000, &msg, sizeof(char));
        co_read(pid, (off_t)0x6004, &p, sizeof(int));
        printf("%i: %c\n", p, msg);
    }

    // finalize
    bsp_end();

    return 0;
}
