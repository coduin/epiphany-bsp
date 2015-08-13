/*
File: e_streaming_dot_product.c

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
#include <common.h>

int main()
{
    bsp_begin();

    int p = bsp_pid();
    int chunk_size = 0;

    char buffer[0x20];
    void *ptr = (void*)&buffer;
    int sizeleft = sizeof(buffer);

    int packets, accum_bytes, status, tag;
    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++)
    {
        // We assume all packet sizes are multiples of 4
        // If not, the cores will crash because of unaligned memory accesses
        bsp_get_tag(&status, &tag);
        bsp_move(ptr, sizeleft);

        if (tag == 1)
            chunk_size = *(int*)ptr;

        sizeleft -= status;
        ptr += status;
    }

    int sum = 0;
    int *ab = 0;
    int counter = 0;

    do {
        ab = ebsp_get_in_chunk();
        for (int i = 0; i < IN_CHUNK_SIZE / sizeof(int) && counter < chunk_size; i += 2, counter += 2) {
            sum += (*ab) * (*(ab+1));
        }
    }
    while (ab != 0);

    // A sync is required between getting messages
    // from host and sending them back
    bsp_sync();

    tag = p;
    ebsp_send_up(&tag, &sum, sizeof(int));

    bsp_end();

    return 0;
}
