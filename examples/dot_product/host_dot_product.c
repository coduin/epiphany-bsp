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

int main(int argc, char** argv) {
    bsp_init("e_dot_product.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // allocate two random vectors of length 512
    int l = 512;
    int* a = (int*)malloc(sizeof(int) * l);
    int* b = (int*)malloc(sizeof(int) * l);
    for (int i = 0; i < l; ++i) {
        a[i] = i;
        b[i] = 2 * i;
    }

    // partition and write to processors
    int chunk = l / bsp_nprocs();
    printf("chunk: %i\n", chunk);

    int tag;
    int tagsize = sizeof(int);
    ebsp_set_tagsize(&tagsize);
    for (int pid = 0; pid < bsp_nprocs(); pid++) {
        tag = 1;
        ebsp_send_down(pid, &tag, &chunk, sizeof(int));
        tag = 2;
        ebsp_send_down(pid, &tag, &a[pid * chunk], sizeof(int) * chunk);
        tag = 3;
        ebsp_send_down(pid, &tag, &b[pid * chunk], sizeof(int) * chunk);
    }

    // enable memory inspector
    // ebsp_inspector_enable();

    // run dotproduct
    ebsp_spmd();

    // read output
    int packets, accum_bytes;
    ebsp_qsize(&packets, &accum_bytes);

    int status;
    int result;
    int sum = 0;
    printf("proc \t partial_sum\n");
    printf("---- \t -----------\n");
    for (int i = 0; i < packets; i++) {
        ebsp_get_tag(&status, &tag);
        ebsp_move(&result, sizeof(int));
        printf("%i: \t %i\n", tag, result);
        sum += result;
    }

    printf("SUM: %i\n", sum);

    free(a);
    free(b);

    // finalize
    bsp_end();

    return 0;
}
