/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
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

#include <stdio.h>
#include <host_bsp.h>

int main(int argc, char** argv) {
    bsp_init("e_bsp_pid.elf", argc, argv);

    int tagsize = sizeof(int);
    ebsp_set_tagsize(&tagsize);

    bsp_begin(bsp_nprocs());
    ebsp_spmd();
    bsp_end();

    int packets, accum_bytes;
    ebsp_qsize(&packets, &accum_bytes);

    int tag;
    int status;
    int result;
    for (int i = 0; i < packets; i++) {
        ebsp_get_tag(&status, &tag);
        ebsp_move(&result, sizeof(int));
        printf("%i: %i", tag, result);
    }

    printf("Done"); // expect: (Done)
}
