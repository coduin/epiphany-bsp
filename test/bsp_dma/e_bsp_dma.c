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

#define RUNCOUNT 4
int bufferTestSizes[RUNCOUNT] = {1, 3, 0x100, 0x400};
#define BUFFERCOUNT 3

int main()
{
    bsp_begin();
    int s = bsp_pid();

    // DMA handles
    ebsp_dma_handle handle[BUFFERCOUNT];

    int globalPass = 1;
    for (int run = 0; run < RUNCOUNT; ++run) {
        int BUFFERSIZE = bufferTestSizes[run];

        // Allocate buffers
        int allocationSucces = 1;

        // Allocate extra space before and after the buffer
        // to check if the dma does not overwrite them.
        // Make sure that the address given to the DMA is
        // 8-byte aligned though.
        char* localbuffer = ebsp_malloc(BUFFERSIZE + 16);
        if (!localbuffer)
            allocationSucces = 0;

        char* remotebuffer[8];
        for (int i = 0; i < BUFFERCOUNT; i++) {
            remotebuffer[i] = ebsp_ext_malloc(BUFFERSIZE);
            if (!remotebuffer[i])
                allocationSucces = 0;
        }

        if (allocationSucces)
        {
            // Fill buffers with data
            for (int i = 0; i < BUFFERSIZE; i++)
                localbuffer[8+i] = 0;
            for (int i = 0; i < BUFFERCOUNT; i++)
                for (int j = 0; j < BUFFERSIZE; j++)
                    remotebuffer[i][j] = (char)(i+1);

            localbuffer[7] = 0xdd;
            localbuffer[8+BUFFERSIZE] = 0xee;

            char valueFirst[BUFFERCOUNT];
            char valueAtEnd[BUFFERCOUNT];

            for (int i = 0; i < BUFFERCOUNT; i++)
                valueFirst[i] = valueAtEnd[i] = 0xff;

            ebsp_barrier();

            // Push some remote->local tasks
            for (int i = 0; i < BUFFERCOUNT; i++)
                ebsp_dma_push(&handle[i], &localbuffer[8], remotebuffer[i], BUFFERSIZE);

            // Wait for the DMAs to finish
            for (int i = 0; i < BUFFERCOUNT; i++) {
                // First check if data is NOT copied too soon
                valueFirst[i] = localbuffer[8+BUFFERSIZE-1];

                // Wait for it to finish
                ebsp_dma_wait(&handle[i]);

                // Check if the data is copied now
                valueAtEnd[i] = localbuffer[8+BUFFERSIZE-1];
            }

            ebsp_barrier();

            // Output results
            for (int i = 0; i < BUFFERCOUNT; i++) {
                if (valueFirst[i] != (char)i) {
                    globalPass = 0;
                    ebsp_message("ERROR: buffer %d copied too soon (contents %d)",
                            i, valueFirst[i]);
                }
            }

            for (int i = 0; i < BUFFERCOUNT; i++) {
                if (valueAtEnd[i] != (char)(i+1)) {
                    globalPass = 0;
                    ebsp_message("ERROR: buffer %d not copied at end", i);
                }
            }

            if (localbuffer[7] != 0xdd || localbuffer[8+BUFFERSIZE] != 0xee) {
                globalPass = 0;
                ebsp_message("ERROR: DMA wrote outside of buffer region");
            }
        }
        else
        {
            globalPass = 0;
            ebsp_message("ERROR: ebsp_malloc(0x%x) or ebsp_ext_malloc(0x%x) failed",
                    BUFFERSIZE, BUFFERSIZE);
        }

        // Free buffers
        if (localbuffer)
            ebsp_free(localbuffer);
        for (int i = 0; i < BUFFERCOUNT; i++)
            if (remotebuffer[i])
                ebsp_free(remotebuffer[i]);
    }
    
    if (globalPass && s == 0)
        ebsp_message("PASS");
    // expect: ($00: PASS)

    bsp_end();

    return 0;
}
