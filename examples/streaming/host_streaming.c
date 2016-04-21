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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char string1[] = "row ";
const char string2[] = "your boat, yes! ";

int main(int argc, char** argv) {
    bsp_init("e_streaming.elf", argc, argv);
    bsp_begin(bsp_nprocs());

    int chunk_size = sizeof(char) * 16;
    int chunks = 6;

    char** upstreams = (char**)malloc(sizeof(char*) * bsp_nprocs());
    char* downdata = (char*)malloc(chunks * chunk_size);
    char* downdataB = (char*)malloc(chunks * chunk_size); 

    int c = 0;
    int index1 = 0;
    int index2 = 0;
    for (int i = chunks * chunk_size / sizeof(char) - 1; i >= 0; --i) {
        downdata[c] = string1[index1++];
        downdataB[c] = string2[index2++];
        c++;

        if (index1 >= strlen(string1))
            index1 = 0;
        if (index2 >= strlen(string2))
            index2 = 0;
    }

    for (int s = 0; s < bsp_nprocs(); ++s) {
        upstreams[s] =
            (char*)ebsp_create_up_stream(s, chunks * chunk_size, chunk_size);
    }

    for (int s = 0; s < bsp_nprocs(); ++s) {
        ebsp_create_down_stream(downdata, s, chunks * chunk_size, chunk_size);
    }

    for (int s = 0; s < bsp_nprocs(); ++s) {
        ebsp_create_down_stream(downdataB, s, chunks * chunk_size, chunk_size);
    }

    ebsp_spmd();

    for (int s = 0; s < bsp_nprocs(); ++s) {
        printf("Result of processor: %i\n", s);
        for (int i = 0; i < chunk_size * chunks / sizeof(char); ++i) {
            printf("%c", upstreams[s][i]);
        }
        printf("\n\n");
    }

    // finalize
    bsp_end();

    return 0;
}
