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

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file contains a very raw implementation of a LU decomposition
// using Epiphany BSP. It only supports small matrices that fit on
// on-core memory, and is meant for demonstrative purposes only.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <host_bsp.h>

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
void ltg(int* i, int* j, int l, int s, int t) {
    (*i) = s + (l / (dim / N)) * N;
    (*j) = t + (l % (dim / M)) * M;
}

// "global to local" index
void gtl(int i, int j, int* l, int* s, int* t) {
    (*s) = i % N;
    (*t) = j % M;
    (*l) = (i / M) * (dim / M) + (j / M);
}

int proc_id(int s, int t) { return s * M + t; }

// multiply AB = C (all n x n)
// assume matrices are stored column-major
void mat_mult(float* A, float* B, float* C, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            C[n * i + j] = 0.0f;
            for (int k = 0; k < n; ++k) {
                C[n * i + j] += A[n * i + k] * B[n * k + j];
            }
        }
    }
}

// permute matrix A (n x n) according to the vector pi (n x 1)
// B = P^T(pi) A))
void mat_permute(int* pi, float* A, float* B, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            B[n * i + j] = A[n * pi[i] + j];
        }
    }
}

// print a matrix to stdout
void mat_pretty_print(float* A, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%.2f\t", A[dim * i + j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    srand(12345);

    // allocate and zero-initialize matrix
    float* A = malloc(sizeof(float) * dim * dim);

    // construct the matrix
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            A[dim * i + j] = rand() % 5 + 1;
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
    for (int i = 0; i < bsp_nprocs(); ++i) {
        ebsp_write(i, &M, (off_t)_LOC_M, sizeof(int));
        ebsp_write(i, &N, (off_t)_LOC_N, sizeof(int));
        ebsp_write(i, &dim, (off_t)_LOC_DIM, sizeof(int));
    }

    int prow = 0;
    int pcol = 0;
    int loc = 0;
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            gtl(i, j, &loc, &prow, &pcol);
            ebsp_write(proc_id(prow, pcol), &A[dim * i + j],
                       _LOC_MATRIX + sizeof(float) * loc, sizeof(float));
        }
    }

// test global to local and local to global function for random processor
#ifdef DEBUG
    int s = 3;
    int t = 3;
    printf("e.g. (s,t) = (3,3): \n");

    int _M, _N, _dim;
    ebsp_read(proc_id(s, t), (off_t)_LOC_M, &_M, sizeof(int));
    ebsp_read(proc_id(s, t), (off_t)_LOC_N, &_N, sizeof(int));
    ebsp_read(proc_id(s, t), (off_t)_LOC_DIM, &_dim, sizeof(int));

    printf("M, N, dim: %i, %i, %i\n", _M, _N, _dim);

    for (int l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
        int i, j;
        ltg(&i, &j, l, s, t);
        float val;
        ebsp_read(proc_id(s, t), _LOC_MATRIX + sizeof(float) * l, &val,
                  sizeof(float));
        printf("%i \t (%i, %i) \t %f \t 0x%x\n", l, i, j, val,
               _LOC_MATRIX + sizeof(float) * l);
    }
#endif

    ebsp_spmd();

    printf("----------------------------: \n");
    printf("Matrix: \n");
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            printf("%.2f\t", A[dim * i + j]);
        }
        printf("\n");
    }

    printf("----------------------------: \n");
    printf("LU decomposition: \n");

    float* Y = malloc(sizeof(float) * dim * dim);

    for (int s = 0; s < N; ++s) {
        for (int t = 0; t < M; ++t) {
            for (int l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
                int i, j;
                ltg(&i, &j, l, s, t);
                ebsp_read(proc_id(s, t), _LOC_MATRIX + sizeof(float) * l,
                          &Y[dim * i + j], sizeof(float));
            }
        }
    }

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            printf("%.2f\t", Y[dim * i + j]);
        }
        printf("\n");
    }

    int* pi = malloc(sizeof(int) * dim);

    printf("PI: \n");
    for (int i = 0; i < dim; ++i) {
        ebsp_read(proc_id(i % N, 0), _LOC_PI + sizeof(int) * (i / N), &pi[i],
                  sizeof(int));
        printf("%i\n", pi[i]);
    }

    printf("----------------------------: \n");

    // we test the results here
    float* L = malloc(sizeof(float) * dim * dim);
    float* U = malloc(sizeof(float) * dim * dim);
    float* B = malloc(sizeof(float) * dim * dim);

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            L[dim * i + j] = 0.0f;
            U[dim * i + j] = 0.0f;
            B[dim * i + j] = 0.0f;
        }
    }

    // obtain L, U from Y
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            if (i == j) {
                U[dim * i + j] = Y[dim * i + j];
                L[dim * i + j] = 1.0f;
            } else if (j < i) {
                L[dim * i + j] = Y[dim * i + j];
            } else {
                U[dim * i + j] = Y[dim * i + j];
            }
        }
    }

    printf("A ---------------------------- \n");
    mat_pretty_print(A, dim);

    printf("Y ---------------------------- \n");
    mat_pretty_print(Y, dim);

    printf("L ---------------------------- \n");
    mat_pretty_print(L, dim);

    printf("U ---------------------------- \n");
    mat_pretty_print(U, dim);

    printf("PA ---------------------------- \n");

    // first see what the permuted A looks like
    mat_permute(pi, A, B, dim);
    mat_pretty_print(B, dim);

    printf("LU ---------------------------- \n");

    // obtain LU
    mat_mult(L, U, B, dim);
    mat_pretty_print(B, dim);

    printf("FINISHED ---------------------- \n");

    // finalize
    bsp_end();

    // free matrices and vectors
    free(A);
    free(L);
    free(U);
    free(B);
    free(pi);

    return 0;
}
