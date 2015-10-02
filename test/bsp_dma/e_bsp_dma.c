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
#include <e-lib.h>
#include "../common.h"

#define BUFFERSIZE 0x1000

int main()
{
    bsp_begin();
    int s = bsp_pid();

    // Allocate local [0] and remote [1] buffer
    char* buffer[2];
    buffer[0] = ebsp_ext_malloc(BUFFERSIZE);
    buffer[1] = ebsp_malloc(BUFFERSIZE);
    
    // Fill buffers with data
    for (int i = 0; i < BUFFERSIZE; i++)
    {
        buffer[0][i] = i & 0xff;
        buffer[1][i] = ~(i & 0xff);
    }

    // DMA handles
    ebsp_dma_handle handle[8];

    // Push some local --> remote transfer tasks
    for (int i = 0; i < 8; i++)
        ebsp_dma_push(&handle[i], buffer[0], buffer[1], BUFFERSIZE);

    // Wait for the DMAs to finish
    for (int i = 0; i < 8; i++)
        ebsp_dma_wait(&handle[i]);

    ebsp_barrier();

    if (s==0)
        ebsp_message("TEST PASS");
    // expect: ($00: TEST PASS)

    bsp_end();

    return 0;
}
