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

#define BUFFERCOUNT 4
#define BUFFERSIZE 0x400

int main()
{
    bsp_begin();
    int s = bsp_pid();

    // Allocate buffers
    char* localbuffer = ebsp_malloc(BUFFERSIZE);
    char* remotebuffer[8];
    for (int i = 0; i < BUFFERCOUNT; i++) {
        remotebuffer[i] = ebsp_ext_malloc(BUFFERSIZE);
    }
    
    // Fill buffers with data
    for (int i = 0; i < BUFFERSIZE; i++)
        localbuffer[i] = 0;
    for (int i = 0; i < BUFFERCOUNT; i++)
        for (int j = 0; j < BUFFERSIZE; j++)
            remotebuffer[i][j] = (char)(i+1);

    char valueFirst[BUFFERCOUNT];
    char valueAtEnd[BUFFERCOUNT];

    for (int i = 0; i < BUFFERCOUNT; i++)
        valueFirst[i] = valueAtEnd[i] = 0xff;

    // DMA handles
    ebsp_dma_handle handle[BUFFERCOUNT];

    if (s == 0)
        ebsp_message("Starting DMA transfers");
        // expect: ($00: Starting DMA transfers)
    ebsp_barrier();

    // Push some remote->local tasks
    for (int i = 0; i < BUFFERCOUNT; i++)
        ebsp_dma_push(&handle[i], localbuffer, remotebuffer[i], BUFFERSIZE);

    // Wait for the DMAs to finish
    for (int i = 0; i < BUFFERCOUNT; i++) {
        // First check if data is NOT copied too soon
        valueFirst[i] = localbuffer[BUFFERSIZE-1];

        // Wait for it to finish
        ebsp_dma_wait(&handle[i]);

        // Check if the data is copied now
        valueAtEnd[i] = localbuffer[BUFFERSIZE-1];
    }

    ebsp_barrier();

    // Output results
    int failed = 0;
    for (int i = 0; i < BUFFERCOUNT; i++) {
        if (valueFirst[i] != (char)i) {
            ebsp_message("ERROR: buffer %d copied too soon (contents %d)", i, valueFirst[i]);
            failed = 1;
        }
    }
    if (!failed && s == 0)
        ebsp_message("PASS: buffers not copied too soon");
    // expect: ($00: PASS: buffers not copied too soon)

    ebsp_barrier();

    failed = 0;
    for (int i = 0; i < BUFFERCOUNT; i++) {
        if (valueAtEnd[i] != (char)(i+1)) {
            ebsp_message("ERROR: buffer %d not copied at end", i);
            failed = 1;
        }
    }
    if (!failed && s == 0)
        ebsp_message("PASS: buffers copied succesfully");
    // expect: ($00: PASS: buffers copied succesfully)

    ebsp_barrier();

    // Test small transfers: first push a large transfer to make
    // sure the DMA is busy. Then attach many small operations
    ebsp_dma_push(&handle[0], localbuffer, remotebuffer[0], BUFFERSIZE);
    for (int i = 1; i < BUFFERCOUNT; i++)
        ebsp_dma_push(&handle[i], localbuffer, remotebuffer[i], 1);

    for (int i = 0; i < BUFFERCOUNT; i++) {
        ebsp_dma_wait(&handle[i]);
    }

    ebsp_barrier();

    if (s == 0)
        ebsp_message("PASS: Test complete");
    // expect: ($00: PASS: Test complete)

    bsp_end();

    return 0;
}
