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

#define BUFFERSIZE 0x3000

int main()
{
    bsp_begin();
    int s = bsp_pid();

    char* buffer[2];
    buffer[0] = ebsp_ext_malloc(BUFFERSIZE);
    buffer[1] = ebsp_malloc(BUFFERSIZE);
    for (int i = 0; i < BUFFERSIZE; i++)
    {
        buffer[0][i] = i & 0xff;
        buffer[1][i] = ~(i & 0xff);
    }

    ebsp_dma_handle handle[8];

    if (s == 0) {
        ebsp_message("Handles at %p, %p, %p, %p, %p, %p, %p, %p",
                &handle[0],
                &handle[1],
                &handle[2],
                &handle[3],
                &handle[4],
                &handle[5],
                &handle[6],
                &handle[7]);
    }
    ebsp_barrier();

    //local --> remote
    for (int i = 0; i < 8; i++)
        ebsp_dma_push(&handle[i], buffer[0], buffer[1], BUFFERSIZE);

    ebsp_barrier();

    ebsp_dma_debug();

    ebsp_barrier();

    ebsp_dma_start(&handle[0]);

    ebsp_dma_debug();

    ebsp_barrier();

    ebsp_dma_debug();

    ebsp_barrier();

    EBSP_MSG_ORDERED("%d", s);
    // expect_for_pid: (pid)

    bsp_end();

    return 0;
}
