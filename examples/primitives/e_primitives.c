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

int n, p;

// Get initial data from the host
// by concatenating all messages after each other into the buffer
// As input, nbytes contains the maximum buffer size
// On return, nbytes contains the amount of bytes that were retrieved
void get_initial_data(void *buffer, unsigned int *nbytes);

// Test allocating and freeing of external memory
int extmem_test();

int main()
{
    float data_buffer[1000];
    float squaresums[16];
    float timings[16];
    float sum;
    float totaltime;
    unsigned int nbytes = sizeof(data_buffer);
    unsigned int data_count;
    float time0, time1, timefix;

    // Initialize
    bsp_begin();

    n = bsp_nprocs(); 
    p = bsp_pid();

    // Get the initial data from the host using the message passing queue
    // Note that this must happen before the first call to bsp_sync
    get_initial_data(&data_buffer, &nbytes);

    // Register two variables
    bsp_push_reg(&squaresums, sizeof(squaresums));
    bsp_sync();
    bsp_push_reg(&time0, sizeof(float));
    bsp_sync();

    // Do computations on data
    // and measure the time

    // First compute the delay between two consecutive calls to bsp_time
    time0 = bsp_time();
    time1 = bsp_time();
    timefix = time1 - time0;

    // Now do the computation
    data_count = nbytes / sizeof(float);
    sum = 0.0f;
    time0 = bsp_time();
    for (int i = 0; i < data_count; i++)
        sum += data_buffer[i] * data_buffer[i];
    time1 = bsp_time();
    time0 = time1 - time0 - timefix;

    // Send result of the sum to processor 0
    // into the correct slot of the array
    bsp_put(0, &sum, &squaresums, p*sizeof(float), sizeof(float));

    // The result of the timings will be transferred
    // through bsp_get instead of bsp_put
    // (only for illustrating how bsp_put and bsp_get work)
    if (p == 0)
        for (int i = 0; i < n; i++)
            bsp_get(i, &time0, 0, &timings[i], sizeof(float));
    bsp_sync();

    // Compute final result on processor 0
    if (p == 0)
    {
        sum = 0.0f;
        for (int i = 0; i < n; i++)
            sum += squaresums[i];
        totaltime = 0.0f;
        for (int i = 0; i < n; i++)
            totaltime += timings[i];
    }

    // Allocate external memory
    time0 = bsp_time();
    int memresult = extmem_test();
    time1 = bsp_time();
    time0 = time1 - time0 - timefix;

    // Everything is done. The only thing left now is sending results
    int tag = 100+p;
    ebsp_send_up(&tag, &time0, sizeof(float));
    if (p == 0)
    {
        // Send back the result of the sum
        tag = 1;
        ebsp_send_up(&tag, &sum, sizeof(float));
        // Send back the result of the memory test
        tag = 2;
        ebsp_send_up(&tag, &memresult, sizeof(int));
        tag = 3;
        ebsp_send_up(&tag, &totaltime, sizeof(float));
    }

    // Finalize
    bsp_end();

    return 0;
}

void get_initial_data(void *buffer, unsigned int *nbytes)
{
    // Get the initial data from the host
    int packets;
    int accum_bytes;
    int tagsize;
    int bufsize = *nbytes;

    bsp_qsize(&packets, &accum_bytes);
    tagsize = ebsp_get_tagsize();

    // Double-check if the host has set the proper tagsize
    if (tagsize != 4)
    {
        bsp_abort("ERROR: tagsize is %d instead of 4", tagsize);
        return;
    }

    // Output some info, but only from core 0 to prevent spamming the console
    int p = bsp_pid();
    if (p == 0)
    {
        ebsp_message("Queue contains %d bytes in %d packet(s).",
                accum_bytes, packets);

        if (accum_bytes > bufsize)
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
        int space = bufsize - offset;
        if (status > space)
            status = space;

        // Get message payload
        bsp_move((void*)((int)buffer + offset), status);
        offset += status;

        if (p==0)
            ebsp_message("Received %d bytes message with tag %d", status, tag);
    }
    *nbytes = offset;
}

int extmem_test()
{
    int errors = 0;

    if (p==0)
        ebsp_message("Memory test: allocating 100 memory objects per core");

    // Allocate external (slow, but larger) memory
    // Use ebsp_ext_malloc and ebsp_free 100 times per core to check if it works
    void* ptrs[100];
    for (int i = 0; i < 100; i++)
        ptrs[i] = ebsp_ext_malloc(1);
    // Now free all odd ones
    for (int i = 0; i < 100; i += 2)
        ebsp_free(ptrs[i]);
    // Allocate them again
    for (int i = 0; i < 100; i += 2)
        ptrs[i] = ebsp_ext_malloc(1);

    ebsp_message("Allocation done");

    // Now we will rotate all the pointers between all cores to check
    // if they are all unique. We have to rotate n-1 times
    // Store the other core's pointers in otherptrs
    void* otherptrs[100];
    bsp_push_reg(&otherptrs, sizeof(otherptrs));
    bsp_sync();
    for (int core = p+1; ; core++)
    {
        if (core == n) core = 0; // wrap around
        if (core == p) break;
        bsp_hpput(core, &ptrs, &otherptrs, 0, sizeof(ptrs));
        bsp_sync();
        // Now check for equality
        for (int j = 0; j < 100; j++)
            if (ptrs[j] == otherptrs[j])
                ++errors;
    }

    if (p==0)
        ebsp_message("Memory test complete (%d)", errors);

    // Free the memory
    for (int i = 0; i < 100; i++)
        ebsp_free(ptrs[i]);

    return errors;
}
