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

    // First send a message indicating that we have started
    ebsp_message("Core %d/%d is running!", p, n);

    // Get the initial data from the host
    int packets;
    int accum_bytes;
    int tagsize;
    bsp_qsize(&packets, &accum_bytes);
    tagsize = ebsp_get_tagsize();

    // Double-check if the host has set the proper tagsize
    if (tagsize != 4)
    {
        if (p==0)
            ebsp_message("ERROR: tagsize is %d instead of 4", tagsize);
        return bsp_end();
    }

    // Output some info, but only from core 0 to prevent spamming the console
    if (p == 0)
    {
        ebsp_message("Queue contains %d bytes in %d packet(s).",
                accum_bytes, packets);
    }

    int status;
    int tag;
    float payload[16];
    
    for (int i = 0; i < packets; i++)
    {
        // Get message tag and size
        bsp_get_tag(&status, &tag);
        if (status == -1)
        {
            ebsp_message("bsp_get_tag failed");
            break;
        }
        if (status != sizeof(float)*16)
        {
            ebsp_message("Message in queue has invalid payload size");
            break;
        }

        // Get message payload
        bsp_move(&payload, sizeof(payload));

        if (p==0)
            ebsp_message("Received %d bytes message with tag %d",
                    status, tag);
    }

    // Register a variable
    float squaresums[16];
    bsp_push_reg(&squaresums, sizeof(squaresums));
    bsp_sync();

    // Do computations
    float squaresum = 0.0f;
    for (int i = 0; i < 16; i++)
        squaresum += payload[i] * payload[i];

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
