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
    // here the messages from the host are available
    int packets = 0;
    int accum_bytes = 0;
    bsp_qsize(&packets, &accum_bytes);

    // test: can obtain messages from host
    EBSP_MSG_ORDERED("packets: %i", packets);
    // expect_for_pid: ("packets: 2")

    int payload_in[2] = {0};
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        bsp_get_tag(&payload_size, &tag_in);
        bsp_move(&payload_in[tag_in], sizeof(int));
    }

    // test: can obtain messages from host
    EBSP_MSG_ORDERED("%i", payload_in[0]);
    // expect_for_pid: (1000 + pid)

    // test: can obtain multiple messages from host
    EBSP_MSG_ORDERED("%i", payload_in[1]);
    // expect_for_pid: (1234)

    // test: gets the correct size from host
    EBSP_MSG_ORDERED("%i", payload_size);
    // expect_for_pid: (4)

    bsp_sync();

    int payload = payload_in[0] + 1000;
    int tag = s;
    ebsp_send_up(&tag, &payload, sizeof(int));

    payload = 3;
    tag = bsp_nprocs() + s;
    ebsp_send_up(&tag, &payload, sizeof(int));

    bsp_end();

    return 0;
}
