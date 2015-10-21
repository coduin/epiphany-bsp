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

#include <e_bsp.h>
#include <stdint.h>
#include <limits.h>
#include "common.h"

void get_block_count(int* n);
void matrix_multiply_add(float* A, float* B, float* C);

int main() {
    bsp_begin();
    int s = bsp_pid();
    int si = s / N;
    int sj = s % N;

    int a_neighbor = si * N + ((sj + 1) % N);
    int b_neighbor = ((si + 1) % N) * N + sj;

    // 5 buffers
    float* a_data[2];
    float* b_data[2];
    float* c_data;

    // neighbor buffer locations
    float* neighbor_a_data[2];
    float* neighbor_b_data[2];

    int M = 0;
    get_block_count(&M);

    int fastmode = 1;

    // Allocate local buffers
    ebsp_open_down_stream((void**)&a_data[0], 0);
    a_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    ebsp_open_down_stream((void**)&b_data[0], 1);
    b_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);

    ebsp_open_up_stream((void**)&c_data, 2);

    // Set C to zero
    for (int i = 0; i < CORE_BLOCK_SIZE * CORE_BLOCK_SIZE; ++i) {
        a_data[1][i] = -1;
        b_data[1][i] = -1;
        c_data[i] = 0;
    }

    // Register their locations
    bsp_push_reg(a_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(a_data[1], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[1], CORE_BLOCK_BYTES);
    bsp_sync();

    // Obtain neighbor locations
    neighbor_a_data[0] = ebsp_get_direct_address(a_neighbor, a_data[0]);
    neighbor_a_data[1] = ebsp_get_direct_address(a_neighbor, a_data[1]);
    neighbor_b_data[0] = ebsp_get_direct_address(b_neighbor, b_data[0]);
    neighbor_b_data[1] = ebsp_get_direct_address(b_neighbor, b_data[1]);

    ebsp_dma_handle dma_handle_a;
    ebsp_dma_handle dma_handle_b;

    // Loop over the blocks (chunks)
    // these are the *global blocks*
    for (int cur_block = 0; cur_block <= M * M * M; cur_block++) {

        if (cur_block != 0) {
            if (cur_block % (M * M) == 0) {
                ebsp_move_down_cursor(1,         // stream id
                                      -(M * M)); // relative chunk count
            } else if (cur_block % M == 0) {
                ebsp_move_down_cursor(0,   // stream id
                                      -M); // relative chunk count
            }
            if (cur_block % M == 0) {
                // Send result of C upwards
                ebsp_barrier();
                ebsp_move_chunk_up((void*)&c_data, 2, fastmode);
                ebsp_barrier();

                if (cur_block == M * M * M) {
                    break;
                }

                // Set C to zero
                for (int i = 0; i < CORE_BLOCK_SIZE * CORE_BLOCK_SIZE; ++i)
                    c_data[i] = 0;
            }
        }

        // Obtain A, B
        ebsp_move_chunk_down((void**)&a_data[0], // address
                             0,                  // stream id
                             0);                 // double buffered mode
        ebsp_move_chunk_down((void**)&b_data[0], // address
                             1,                  // stream id
                             0);                 // double buffered mode

        int cur = 0;        // computation
        int cur_buffer = 1; // data transfer

        // Multiply this block, by looping over the *core blocks*
        for (int i = 0; i < N; i++) {
            if (i != N - 1) {
                ebsp_dma_push(&dma_handle_a, neighbor_a_data[cur_buffer],
                              a_data[cur], CORE_BLOCK_BYTES);
                ebsp_dma_push(&dma_handle_b, neighbor_b_data[cur_buffer],
                              b_data[cur], CORE_BLOCK_BYTES);
            }

            // Perform C += A * B
            matrix_multiply_add(a_data[cur], b_data[cur], c_data);

            if (i == N - 1)
                break;

            // Switch buffers
            cur_buffer = 1 - cur_buffer;
            cur = 1 - cur;

            ebsp_dma_wait(&dma_handle_b);
            ebsp_dma_wait(&dma_handle_a);

            ebsp_barrier();
        }
    }

    ebsp_barrier();
    ebsp_close_down_stream(0);
    ebsp_close_down_stream(1);
    ebsp_close_up_stream(2);

    bsp_end();
}

void get_block_count(int* M) {
    int packets = 0;
    int accum_bytes = 0;
    int status = 0;
    int tag = 0;

    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++) {
        bsp_get_tag(&status, &tag);
        if (tag == 1)
            bsp_move(M, sizeof(int));
    }
}

void matrix_multiply_add(float* A, float* B, float* C) {
    for (int i = 0; i < CORE_BLOCK_SIZE; i++)
        for (int j = 0; j < CORE_BLOCK_SIZE; j++)
            for (int k = 0; k < CORE_BLOCK_SIZE; k++)
                C[i * CORE_BLOCK_SIZE + j] +=
                    A[i * CORE_BLOCK_SIZE + k] * B[k * CORE_BLOCK_SIZE + j];
}
