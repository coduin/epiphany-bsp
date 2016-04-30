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

#include <host_bsp.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    bsp_init("e_bsp_vertical_mp.elf", argc, argv);
    bsp_begin(bsp_nprocs());

    int n = bsp_nprocs();

    int tagsize = sizeof(int);
    ebsp_set_tagsize(&tagsize);

    int tag = 0;
    int payload = 0;
    for (int s = 0; s < n; ++s) {
        tag = 0;
        payload = 1000 + s;
        ebsp_send_down(s, &tag, &payload, sizeof(int));

        tag = 1;
        payload = 1234;
        ebsp_send_down(s, &tag, &payload, sizeof(int));
    }

    ebsp_spmd();

    int packets = 0;
    int accum_bytes = 0;
    ebsp_qsize(&packets, &accum_bytes);

    printf("packets: %i\n", packets);
    // FIXME nprocs
    // expect: (packets: 32)

    int* payloads = malloc(2 * n * sizeof(int));
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        ebsp_get_tag(&payload_size, &tag_in);
        ebsp_move(&payloads[tag_in], sizeof(int));
    }

    for (int i = 0; i < n; ++i) {
        printf("$%02d: %i\n", i, payloads[i]);
        // expect_for_pid: (2000 + pid)
    }

    for (int i = n; i < 2 * n; ++i) {
        printf("$%02d: %i\n", i - bsp_nprocs(), payloads[i]);
        // expect_for_pid: (3)
    }

    free(payloads);

    bsp_end();

    return 0;
}
