/*
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
#include "../common.h"

int main() {
    bsp_begin();
    int s = bsp_pid();
    int p = bsp_nprocs();

    // test: can set tagsize from host
    int tagsize = ebsp_get_tagsize(tagsize);

    EBSP_MSG_ORDERED("%i", tagsize);
    // expect_for_pid: (4)

    bsp_sync();

    tagsize = 8;
    bsp_set_tagsize(&tagsize);
    bsp_sync();

    // test: can set and get tagsize from core
    EBSP_MSG_ORDERED("%i", ebsp_get_tagsize());
    // expect_for_pid: (8)

    // test: old value for tagsize gets written back to arg
    EBSP_MSG_ORDERED("%i", tagsize);
    // expect_for_pid: (4)

    int tag = 0;
    int payload = 0;
    bsp_send((s + 1) % p, &tag, &payload, sizeof(int));
    bsp_send((s + 2) % p, &tag, &payload, sizeof(int));
    bsp_sync();

    int packets = 0;
    int accum_bytes = 0;
    bsp_qsize(&packets, &accum_bytes);

    // test: can obtain the number of packages
    EBSP_MSG_ORDERED("%i", packets);
    // expect_for_pid: (2)

    // test: can obtain the number of total bytes
    EBSP_MSG_ORDERED("%i", accum_bytes);
    // expect_for_pid: (8)

    bsp_sync();

    tag = 2;
    payload = 42;
    bsp_send((s + 1) % p, &tag, &payload, sizeof(int));
    bsp_sync();

    bsp_qsize(&packets, &accum_bytes);

    int payload_in = 0;
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        bsp_get_tag(&payload_size, &tag_in);
        bsp_move(&payload_in, sizeof(int));

        // test: can send messages
        EBSP_MSG_ORDERED("%i", payload_in);
        // expect_for_pid: (42)

        // test: get the correct tag
        EBSP_MSG_ORDERED("%i", tag_in);
        // expect_for_pid: (2)
    }

    bsp_end();

    return 0;
}
