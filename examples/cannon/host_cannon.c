#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void print_matrix(float* A, int matrix_size);

float* C = 0;
float* out_streams[N * N] = {0};

void sync_callback();

// Initial total matrix
int k = 0;
int matrix_size = 0;
int matrix_bytes = 0;

int block_count = 0;

int main(int argc, char **argv)
{
    k = 4;
    matrix_size = 1 << 4;
    matrix_bytes = matrix_size * matrix_size * sizeof(float);
    block_count = matrix_size / BLOCK_SIZE;

    printf("%d X %d cores\n", N, N);
    printf("core_blocks: %d X %d = %d bytes = 0x%x bytes\n", CORE_BLOCK_SIZE, CORE_BLOCK_SIZE, CORE_BLOCK_BYTES, CORE_BLOCK_BYTES);
    printf("blocks: %d X %d = %d bytes = 0x%x bytes\n", BLOCK_SIZE, BLOCK_SIZE, BLOCK_BYTES, BLOCK_BYTES);
    printf("full matrix: %d X %d = %d bytes = 0x%x bytes\n", matrix_size, matrix_size, matrix_bytes, matrix_bytes); 
    // Prepare full matrix
    float* A = malloc(matrix_bytes);
    float* B = malloc(matrix_bytes);
    C = malloc(matrix_bytes);

    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            A[i * matrix_size + j] = (float)i;
            B[i * matrix_size + j] = (float)j;
            C[i * matrix_size + j] = 0;
        }
    }

    printf("Initial matrix A:\n");
    print_matrix(A, matrix_size);
    printf("Initial matrix B:\n");
    print_matrix(B, matrix_size);
    
    //
    // Partition into stream
    //
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
            // A[ (block_Y * BLOCK_SIZE + j) * matrix_size + (block_X * BLOCK_SIZE + j) ]
            //
            // Within this block, we want to partition into 16 * 16 smaller blocks

            for (int i = 0; i < BLOCK_SIZE; i++) {
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    float element_A = A[ (block_Y * BLOCK_SIZE + i) * matrix_size + (block_X * BLOCK_SIZE + j) ];
                    float element_B = B[ (block_X * BLOCK_SIZE + i) * matrix_size + (block_Y * BLOCK_SIZE + j) ];
                    // i,j are coordinates within the block
                    int X = i / CORE_BLOCK_SIZE;
                    int Y = j / CORE_BLOCK_SIZE;
                    //
                    // Target processor: P(i,j) gets block_A(i,i+j) and block_B(i+j,j)
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
    printf("\a\n");
    bsp_init("e_cannon.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    ebsp_set_sync_callback(sync_callback);

    int tagsize = 4;
    int tag = 1;
    ebsp_set_tagsize(&tagsize);

    for (int s = 0; s < N * N; s++) {
        ebsp_send_buffered(stream_A[s], s, matrix_bytes / (N * N), CORE_BLOCK_BYTES);
        ebsp_send_buffered(stream_B[s], s, matrix_bytes / (N * N), CORE_BLOCK_BYTES);
        ebsp_send_down(s, &tag, &matrix_size, sizeof(int));
        //ebsp_open_out_stream(s, // core id
        //        CORE_BLOCK_BYTES, // stream size
        //        &out_streams[s] ); // pointer
    }

    printf("Starting spmd\n");
    ebsp_spmd();
    printf("Finished spmd\n");

    // Gather C
    // Loop over blocks
    int cur_index[N * N];
    for (int s = 0; s < N * N; s++)
        cur_index[s] = 0;

    //for (int block_Y = 0; block_Y < block_count; block_Y++) {
    //    for (int block_X = 0; block_X < block_count; block_X++) {
    //        for (int s = 0; s < N * N; s++) {
    //            int s_i = s / N;
    //            int s_j = s % N;
    //            int c_i = s_i * CORE_BLOCK_SIZE;
    //            int c_j = s_j * CORE_BLOCK_SIZE;
    //            for (int i = 0; i < CORE_BLOCK_SIZE; i++) {
    //                for (int j = 0; j < CORE_BLOCK_SIZE; j++) {
    //                    C[ (block_Y * BLOCK_SIZE + c_i + i) * matrix_size + (block_X * BLOCK_SIZE + c_j + j) ] = out_streams[s][cur_index[s]++];
    //                }
    //            }
    //        }
    //    }
    //}

    bsp_end();
    printf("\a\n");

    free(A);
    free(B);
}

void sync_callback()
{
    printf("Host syncing");
    for (int i = 0; i < 5; i++) {
        printf(".");
        fflush(stdout);
        usleep(100000);
    }
    printf("\n");
}

void print_matrix(float* A, int matrix_size)
{
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            printf("%5.2f ", A[i * matrix_size + j]);
        }
        printf("\n");
    }
    printf("\n");
}

