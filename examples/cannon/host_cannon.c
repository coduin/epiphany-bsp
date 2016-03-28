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

// This example contains an implementation of Cannon's algorithm.
// See also: <https://en.wikipedia.org/wiki/Cannon's_algorithm>.

#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define __USE_XOPEN2K
#define __USE_POSIX199309 1
#include <time.h>

void print_matrix(float* A, int matrix_size);

float* C = 0;
float* up_streams[N * N] = {0};

// Initial total matrix
int matrix_size = 0;
int matrix_bytes = 0;

int block_count = 0;

int main(int argc, char** argv) {
    //matrix_size = BLOCK_SIZE * 8;
<<<<<<< HEAD
    matrix_size = SCRIPT_MATRIX_SIZE;
=======
    matrix_size = 360;
>>>>>>> 04ee2f4766a7a0931b48469f292e0942e6ed8abd
    int M = matrix_size / BLOCK_SIZE;
    matrix_bytes = matrix_size * matrix_size * sizeof(float);
    block_count = matrix_size / BLOCK_SIZE;

    printf("Multiplying two %i x %i matrices\n", matrix_size, matrix_size);
    printf("Full matrix consists of %dx%d = %d superblocks of size %dx%d\n",
            M, M, M * M, BLOCK_SIZE, BLOCK_SIZE);
    printf("One superblock contains %d core-blocks of size %dx%d\n",
            N * N, CORE_BLOCK_SIZE, CORE_BLOCK_SIZE);

    // Prepare full matrix
    float* A = malloc(matrix_bytes);
    float* B = malloc(matrix_bytes);
    C = malloc(matrix_bytes);

    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            A[i * matrix_size + j] = (float)i;
            B[i * matrix_size + j] = (float)j;
            C[i * matrix_size + j] = 0.0f;
        }
    }

    // Partition into stream
    float* stream_A[N * N];
    float* stream_B[N * N];
    int cur_index_A[N * N];
    int cur_index_B[N * N];

    for (int i = 0; i < N * N; i++) {
        cur_index_A[i] = 0;
        cur_index_B[i] = 0;
        stream_A[i] = malloc(matrix_bytes / (N * N));
        stream_B[i] = malloc(matrix_bytes / (N * N));
    }

    // Loop over blocks
    for (int block_Y = 0; block_Y < block_count; block_Y++) {
        for (int block_X = 0; block_X < block_count; block_X++) {
            // When A is divided in BLOCK_SIZE * BLOCK_SIZE blocks
            // we want the block at block_Y, block_X
            // So that block's top-left element is
            // (i,j) = (block_Y * BLOCK_SIZE , block_X * BLOCK_SIZE)
            // Now loop i,j from 0 to BLOCK_SIZE and use
            // A[ (block_Y * BLOCK_SIZE + j) * matrix_size + (block_X *
            // BLOCK_SIZE + j) ]
            //
            // Within this block, we want to partition into 16 * 16 smaller
            // blocks

            for (int i = 0; i < BLOCK_SIZE; i++) {
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    float element_A =
                        A[(block_Y * BLOCK_SIZE + i) * matrix_size +
                          (block_X * BLOCK_SIZE + j)];
                    float element_B =
                        B[(block_X * BLOCK_SIZE + i) * matrix_size +
                          (block_Y * BLOCK_SIZE + j)];
                    // i,j are coordinates within the block
                    int X = i / CORE_BLOCK_SIZE;
                    int Y = j / CORE_BLOCK_SIZE;
                    //
                    // Target processor: P(i,j) gets block_A(i,i+j) and
                    // block_B(i+j,j)
                    //
                    // block_A(X,Y) goes to P(X,Y-X)
                    // block_B(X,Y) goes to P(X-Y,Y)
                    //
                    // P(i,j) has PID 4*i + j
                    int pidA = 4 * X + ((Y - X + N) % N);
                    int pidB = 4 * ((X - Y + N) % N) + Y;

                    stream_A[pidA][cur_index_A[pidA]++] = element_A;
                    stream_B[pidB][cur_index_B[pidB]++] = element_B;
                }
            }
        }
    }

    // Initialize the BSP system
    bsp_init("e_cannon.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    int tagsize = 4;
    int tag = 1;
    ebsp_set_tagsize(&tagsize);

    for (int s = 0; s < N * N; s++) {
        ebsp_create_down_stream(stream_A[s], s, matrix_bytes / (N * N),
                                CORE_BLOCK_BYTES);
        ebsp_create_down_stream(stream_B[s], s, matrix_bytes / (N * N),
                                CORE_BLOCK_BYTES);
        ebsp_send_down(s, &tag, &M, sizeof(int));
        up_streams[s] = ebsp_create_up_stream(
            s,                                            // core id
            block_count * block_count * CORE_BLOCK_BYTES, // total size
            CORE_BLOCK_BYTES);                            // chunk size
    }

    // Timer
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    ebsp_spmd();
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    float time_elapsed =
        (ts_end.tv_sec - ts_start.tv_sec +
         (ts_end.tv_nsec - ts_start.tv_nsec) * 1.0e-9);
    printf("ebsp_spmd() time in seconds: %f\n", time_elapsed);

    // Gather C
    // Loop over blocks
    // everything in row-major order
    int cur_index[N * N] = {0};
    for (int block = 0; block < block_count * block_count; ++block) {
        int blockI = block / block_count;
        int blockJ = block % block_count;
        int baseColumn = blockJ * BLOCK_SIZE;
        int baseRow = blockI * BLOCK_SIZE;
        for (int proc = 0; proc < N * N; ++proc) {
            int s = proc / N;
            int t = proc % N;
            int coreBlockColumn = baseColumn + t * CORE_BLOCK_SIZE;
            int coreBlockRow = baseRow + s * CORE_BLOCK_SIZE;
            for (int i = 0; i < CORE_BLOCK_SIZE; ++i) {
                for (int j = 0; j < CORE_BLOCK_SIZE; ++j) {
                    C[(coreBlockRow + i) * matrix_size + coreBlockColumn + j] =
                        up_streams[proc][cur_index[proc]++];
                }
            }
        }
    }

    // Uncomment this line to view the entire matrix
    // print_matrix(C, matrix_size);
    
    printf("Result: C[n - 1, n - 1] = %.2f\n", C[matrix_size * matrix_size - 1]);

    bsp_end();

    free(A);
    free(B);
}

void print_matrix(float* A, int matrix_size) {
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            printf("%5.2f ", A[i * matrix_size + j]);
        }
        printf("\n");
    }
    printf("\n");
}
