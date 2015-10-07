/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
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

#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>

// Data to be processed by the epiphany cores
#define data_count (16 * 1000)
float data[data_count];

int nprocs;

void generate_data();
void send_data();
void retrieve_data();

int main(int argc, char** argv) {
    // Initialize the BSP system
    if (!bsp_init("e_primitives.srec", argc, argv)) {
        fprintf(stderr, "ERROR: bsp_init() failed\n");
        return -1;
    }

    // Get the number of processors available
    int nprocs_available = bsp_nprocs();

    printf("Amount of cores available: %i\n", nprocs_available);
    for (;;) {
        printf("Enter the amount of cores to use: ");
        nprocs = 0;
        if (scanf("%d", &nprocs) < 0)
            return 1;
        if (nprocs <= 0 || nprocs > nprocs_available)
            printf("Invalid. Enter a number between 1 and %d\n",
                   nprocs_available);
        else
            break;
    }

    // Initialize the epiphany system, and load the e-program
    if (!bsp_begin(nprocs)) {
        fprintf(stderr, "ERROR: bsp_begin() failed\n");
        return -1;
    }

    // Send some initial data to the processors (i.e. matrices)
    generate_data();
    send_data();

    // Run the SPMD on the e-cores
    ebsp_spmd();

    printf("Retrieving results\n");

    // Retrieve results
    retrieve_data();

    // Finalize
    bsp_end();
}

void generate_data() {
    for (int i = 0; i < data_count; i++)
        data[i] = (float)(1 + i);
}

void send_data() {
    // Give it a tag. For example an integer
    int tag;
    int tagsize = sizeof(int);

    // Send the data
    // We divide it in nprocs parts
    int chunk_size = (data_count + nprocs - 1) / nprocs;

    ebsp_set_tagsize(&tagsize);
    for (int p = 0; p < nprocs; p++) {
        tag = 100 + p; // random tag
        ebsp_send_down(p, &tag, &data[p * chunk_size],
                       sizeof(float) * chunk_size);
    }
}

void retrieve_data() {
    int packets;
    int accum_bytes;
    int tagsize;

    ebsp_qsize(&packets, &accum_bytes);
    tagsize = ebsp_get_tagsize();

    printf("Queue contains %d bytes in %d packet(s), tagsize %d\n", accum_bytes,
           packets, tagsize);

    void* tag = malloc(tagsize);
    int status;

    for (int i = 0; i < packets; i++) {
        ebsp_get_tag(&status, tag);
        if (status == -1) {
            printf("bsp_get_tag failed");
            break;
        }
        int ntag = *(int*)tag;
        if (ntag == 1) {
            float value;
            ebsp_move(&value, sizeof(float));
            printf("Result 1: square sum is %f\n", value);
        } else if (ntag == 2) {
            int value;
            ebsp_move(&value, sizeof(int));
            printf("Result 2: memory allocation errors: %d\n", value);
        } else if (ntag == 3) {
            float value;
            ebsp_move(&value, sizeof(float));
            printf("Result 3: total squaresum time of all cores: %e\n", value);
        } else if (ntag >= 100 && ntag <= 200) {
            float value;
            ebsp_move(&value, sizeof(float));
            printf("Result 4: memory allocation time for core %d: %e\n",
                   ntag - 100, value);
        } else {
            printf("Received %d bytes with tag %d:\n", status, ntag);
        }
    }
    free(tag);
}
