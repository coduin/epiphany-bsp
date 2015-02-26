/*
File: host_dot_product.c

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
#include <host_bsp_inspector.h>
#include <stdlib.h>
#include <stdio.h> 

int main(int argc, char **argv)
{
    bsp_init("bin/e_dot_product.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // for loops
    int pid = 0;
    int i;

    // allocate two random vectors of length 512
    int l = 512;
    int* a = (int*)malloc(sizeof(int) * l);
    int* b = (int*)malloc(sizeof(int) * l);
    for(i = 0; i < l; ++i) {
        a[i] = i;
        b[i] = 2*i;
    }

    // partition and write to processors
    int chunk = l / bsp_nprocs();
    printf("chunk: %i\n", chunk);

    // write in memory bank 4
    int base = 0x4000;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        int prow, pcol;
        co_write(pid, &chunk, (off_t)base, sizeof(int));
        for(i = 0; i < chunk; ++i) {
            co_write(pid, &a[i + chunk*pid], (off_t)(base + 4 + 4*i), sizeof(int));
            co_write(pid, &b[i + chunk*pid], (off_t)(base + 4 + 4*chunk + 4*i), sizeof(int));
        }
    }
    
    // enable memory inspector
    ebsp_inspector_enable();

    // run dotproduct
    ebsp_spmd();

    // read output
    int* result = malloc(sizeof(int) * bsp_nprocs());
    int sum = 0;
    printf("proc \t mem_loc \t partial_sum\n");
    printf("---- \t ------- \t -----------\n");
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        co_read(pid, (off_t)(base + 4 + 8*chunk), &result[pid], sizeof(int));
        printf("%i: \t 0x%x \t %i\n", pid, base + 4 + 8*chunk, result[pid]);
        sum += result[pid];
    }

    printf("SUM: %i\n", sum);

    free(a);
    free(b);
    free(result);

    // finalize
    bsp_end();

    return 0;
}
