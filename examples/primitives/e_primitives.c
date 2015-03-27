/*
File: e_primitives.c

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
#include "e-lib.h"

int main()
{
    // Initialize
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    // Get the initial data from the host
    int packets;
    int accum_bytes;
    int tagsize;
    const int data_count = 1000;
    float data_buffer[data_count];

    bsp_qsize(&packets, &accum_bytes);
    tagsize = ebsp_get_tagsize();

    // Double-check if the host has set the proper tagsize
    if (tagsize != 4)
    {
        bsp_abort("ERROR: tagsize is %d instead of 4", tagsize);
        return 0;
    }

    // Output some info, but only from core 0 to prevent spamming the console
    if (p == 0)
    {
        ebsp_message("Queue contains %d bytes in %d packet(s).",
                accum_bytes, packets);

        if (accum_bytes > sizeof(float)*data_count)
            ebsp_message("Received more bytes than local buffer could hold.");
    }

    int status;
    int tag;
    int offset = 0;
    
    for (int i = 0; i < packets; i++)
    {
        // Get message tag and size
        bsp_get_tag(&status, &tag);
        if (status == -1)
        {
            ebsp_message("bsp_get_tag failed");
            break;
        }
        // Truncate everything that does not fit
        if (offset + status > sizeof(float)*data_count)
            status = sizeof(float)*data_count - offset;

        // Get message payload
        bsp_move(&data_buffer[offset], status);
        offset += status;

        if (p==0)
            ebsp_message("Received %d bytes message with tag %d",
                    status, tag);
    }
    
    int received_count = offset / sizeof(float);

    // Register a variable
    float squaresums[16];
    bsp_push_reg(&squaresums, sizeof(squaresums));
    bsp_sync();

    float squaresum = 0.0f;
    for (int i = 0; i < received_count; i++)
        squaresum += data_buffer[i] * data_buffer[i];

    // Send result to processor 0
    bsp_hpput(0, &squaresum, &squaresums, p*sizeof(float), sizeof(float));
    bsp_sync();
    
    if (p == 0)
    {
        squaresum = 0.0f;
        for (int i = 0; i < 16; i++)
            squaresum += squaresums[i];

        ebsp_message("Total square sum is %f", squaresum);
    }

    // Finalize
    bsp_end();

    return 0;
}
