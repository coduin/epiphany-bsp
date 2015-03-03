/*
File: host_lu_decomposition.h

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

#include "common.h"

#define DEBUG

// information on matrix and procs
int N = -1;
int M = -1;

// always choose multiple of 4 such that we dont have to worry
// about heterogeneous distributions too much,
// which makes a lot of things much easier
int dim = 20;

// "local to global" index
int ltg(int* i, int* j, int l, int s, int t)
{
    (*i) = s + (l / (dim / N)) * N;
    (*j) = t + (l % (dim / M)) * M;
}

// "global to local" index
int gtl(int i, int j, int* l, int* s, int* t)
{
    (*s) = i % N;
    (*t) = j % M;
    (*l) = (i / M) * (dim / M) + (j / M);
}

int proc_id(int s, int t)
{
    return s * M + t;
}

int main(int argc, char **argv)
{
    // allocate and zero-initialize matrix
    float* mat = malloc(sizeof(float) * dim * dim);

    // construct the matrix
    int i = 0; 
    int j = 0;
    for(i = 0; i < dim; ++i) {
        for(j = 0; j < dim; ++j) {
            if(i > j) 
                mat[dim*i + j] = (float)(i + 1) / (j+1);
            else 
                mat[dim*i + j] = (float)(j + 1) / (i+1);
//            if(i == j)
//                mat[dim * i + j] = 1.0f;
//            else
//                mat[dim * i + j] = 0.0f;
        }
    }

    // initialize the BSP system
    bsp_init("bin/e_lu_decomposition.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // distribute the matrix
    switch (bsp_nprocs()) {
        case 16:
            N = 4;
            M = 4;
            break;

        case 64:
            N = 8;
            M = 8;
            break;

        default:
            fprintf(stderr, "Unsupported processor count, please add values\
                    for N and M in the host program.");
            return -1;
    }

    printf("LUD: Writing info on procs and matrix \n");
    // Write M, N and dim to every processor such that they can figure out 
    // the (s,t) pair, and gtl / ltg functions
    for(i = 0; i < bsp_nprocs(); ++i) {
        ebsp_write(i, &M, (off_t)LOC_M, sizeof(int));
        ebsp_write(i, &N, (off_t)LOC_N, sizeof(int));
        ebsp_write(i, &dim, (off_t)LOC_DIM, sizeof(int));
    }

    int s = 0;
    int t = 0;
    int l = 0;
    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            gtl(i, j, &l, &s, &t);
            ebsp_write(proc_id(s, t),
                    &mat[dim*i + j],
                    LOC_MATRIX + sizeof(float) * l,
                    sizeof(float));
        }
    }

    // test global to local and local to global function for random processor
#ifdef DEBUG
    s = 3;
    t = 3;
    printf("e.g. (s,t) = (3,3): \n");

    int _M, _N, _dim;
    ebsp_read(proc_id(s, t), (off_t)LOC_M, &_M, sizeof(int));
    ebsp_read(proc_id(s, t), (off_t)LOC_N, &_N, sizeof(int));
    ebsp_read(proc_id(s, t), (off_t)LOC_DIM, &_dim, sizeof(int));

    printf("M, N, dim: %i, %i, %i\n", _M, _N, _dim);

    for (l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
            ltg(&i, &j, l, s, t);
            float val;
            ebsp_read(proc_id(s, t),
                    LOC_MATRIX + sizeof(float) * l,
                    &val,
                    sizeof(float));
            printf("%i \t (%i, %i) \t %f \t 0x%x\n",
                    l, i, j, val,
                    LOC_MATRIX + sizeof(float) * l);
    }
#endif

#ifdef DEBUG
    //ebsp_inspector_enable();
#endif

    ebsp_spmd();

    printf("----------------------------: \n");
    printf("Matrix: \n");
    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            printf("%.2f\t", mat[dim * i + j]);
        }
        printf("\n");
    }

    printf("----------------------------: \n");
    printf("LU decomposition: \n");

    for (s = 0; s < N; ++s) {
        for (t = 0; t < M; ++t) {
            for (l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
                    ltg(&i, &j, l, s, t);
                    ebsp_read(proc_id(s, t),
                            LOC_MATRIX + sizeof(float) * l,
                            &mat[dim*i + j], sizeof(float));
            }
        }
    }

    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            printf("%.2f\t", mat[dim * i + j]);
        }
        printf("\n");
    }


    printf("PI: \n");
    for(int i = 0; i < dim; ++i) {
        int val = 0;
        ebsp_read(proc_id(i % N, 0),
                LOC_PI + sizeof(int) * (i / N),
                &val, sizeof(int));
        printf("%i\n", val);
    }

    printf("----------------------------: \n");

    // want to test here

    // finalize
    bsp_end();

    return 0;
}
