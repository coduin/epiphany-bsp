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
    const int max_data_count = 1000;
    const int max_data_bytes = sizeof(float)*max_data_count;
    float data_buffer[max_data_count];

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

        if (accum_bytes > max_data_bytes)
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
        // Note that local variables will be optimized into registers
        // so do not worry about using an excessive amount of local variables
        // since there are more than 50 non-reserved registers available
        int space = max_data_bytes - offset;
        if (status > space)
            status = space;

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

    float sum = 0.0f;
    for (int i = 0; i < received_count; i++)
        sum += data_buffer[i] * data_buffer[i];

    // Send result to processor 0
    bsp_hpput(0, &sum, &squaresums, p*sizeof(float), sizeof(float));
    bsp_sync();
    
    if (p == 0)
    {
        sum = 0.0f;
        for (int i = 0; i < 16; i++)
            sum += squaresums[i];

        ebsp_message("Total square sum is %f", squaresum);
    }

    // Finalize
    bsp_end();

    return 0;
}
