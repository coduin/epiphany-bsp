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

#define RUNCOUNT 12
int bufferTestSizes[RUNCOUNT] = {1, 2, 3, 4, 5, 6, 7, 8, 0x100,
    0x1000, 0x2000, 0x9000};

int main()
{
    bsp_begin();
    int s = bsp_pid();

    int globalPass = 1;
    for (int run = 0; run < RUNCOUNT; ++run) {
        int BUFFERSIZE = bufferTestSizes[run];

        // Allocate buffers
        char* localbuffer = ebsp_malloc(BUFFERSIZE);
        char* remotebuffer = ebsp_ext_malloc(BUFFERSIZE); // slow

        // Write to start and end of buffer to confirm
        // locatations are writable

        if (localbuffer) {
            localbuffer[0] = 1;
            localbuffer[BUFFERSIZE - 1] = 2;
        }
        if (remotebuffer) {
            remotebuffer[0] = 3;
            remotebuffer[BUFFERSIZE - 1] = 4;
        }

        ebsp_barrier();

        // Read from buffers to confirm they are readable
        char total = 0;
        if (localbuffer) {
            total += localbuffer[0];
            total += localbuffer[BUFFERSIZE - 1];
        }
        if (remotebuffer) {
            total += remotebuffer[0];
            total += remotebuffer[BUFFERSIZE - 1];
        }

        char expectedtotal = 0;
        if (BUFFERSIZE == 1) {
            expectedtotal = 12; // because buffer[0] = buffer[N - 1]
        } else {
            if (localbuffer)
                expectedtotal += 3;
            if (remotebuffer)
                expectedtotal += 7;
        }

        if (total != expectedtotal) {
            globalPass = 0;
            ebsp_message("ERROR: At BUFFERSIZE = %d = 0x%x, 'total' is %d",
                    BUFFERSIZE, BUFFERSIZE, total);
        }

        // Free buffers
        if (localbuffer) ebsp_free(localbuffer);
        if (remotebuffer) ebsp_free(remotebuffer);
    }
    
    if (s == 0)
        ebsp_message(globalPass ? "PASS" : "FAIL");
    // expect: ($00: PASS)

    bsp_end();

    return 0;
}
