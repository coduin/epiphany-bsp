/*
File: host_out_stream.c

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
#include <stdlib.h>

int main(int argc, char **argv)
{
    bsp_init("e_streaming.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    int chunk_size = sizeof(float) * 16;
    int chunks = 4;

    float** upstreams = (float**)malloc(sizeof(float*) * bsp_nprocs());
    float* downdata =  (float*)malloc(chunks * chunk_size);
    float* downdataB =  (float*)malloc(chunks * chunk_size);

    int c = 0;
    for (int i = chunks * chunk_size / sizeof(float) - 1;
            i >= 0; --i) {
        downdata[c] = i;
        downdataB[c] = c;
        c++;
    }
   
    for (int s = 0; s < bsp_nprocs(); ++s) {
        upstreams[s] = (float*)ebsp_create_up_stream(s,
                chunks * chunk_size,
                chunk_size);
    }

    for (int s = 0; s < bsp_nprocs(); ++s) {
        ebsp_create_down_stream(downdata,
                s,
                chunks * chunk_size,
                chunk_size);
    }

    for (int s = 0; s < bsp_nprocs(); ++s) {
        ebsp_create_down_stream(downdataB,
                s,
                chunks * chunk_size,
                chunk_size);
    }

    ebsp_spmd();

    for (int s = 0; s < bsp_nprocs(); ++s) {
        printf("proc: %i\n", s);
        for (int i = 0; i < chunk_size * chunks / sizeof(float); ++i) {
            printf("%.2f ", upstreams[s][i]);
        }
        printf("\n");
    }

    // finalize
    bsp_end();

    return 0;
}

