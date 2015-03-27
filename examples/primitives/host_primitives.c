/*
File: host_primitives.c

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

#include <host_bsp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // Initialize the BSP system
    if (!bsp_init("bin/e_primitives.srec", argc, argv))
    {
        fprintf(stderr, "ERROR: bsp_init() failed\n");
        return -1;
    }

    // Show the number of processors available
    printf("bsp_nprocs(): %i\n", bsp_nprocs());

    // Initialize the epiphany system, and load the e-program
    if (!bsp_begin(bsp_nprocs()))
    {
        fprintf(stderr, "ERROR: bsp_begin() failed\n");
        return -1;
    }

    //
    // Send some initial data to the processors
    // (matrix data for example)
    //

    // Give it a tag. For example an integer
    int tag;
    int tagsize = sizeof(int);

    // Payload
    float data[16][16];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            data[i][j] = (float)(i+j+1);

    // Send the data
    ebsp_set_tagsize(&tagsize);
    for (int i = 0; i < bsp_nprocs(); i++)
    {
        tag = i;
        ebsp_senddown(i, &tag, &data[i][0], 16*sizeof(float));
    }

    // Run the SPMD on the e-cores
    ebsp_spmd();

    // Finalize
    bsp_end();
}
