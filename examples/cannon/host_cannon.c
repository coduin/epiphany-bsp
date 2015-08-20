#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>

void print_matrix(float* A, int matrix_size);

// N * N cores
#define N 4

int main(int argc, char **argv)
{
    const int core_block_size = 2; // max 32
    const int core_block_bytes = core_block_size * core_block_size * sizeof(float);
    const int block_size = N * core_block_size;
    const int block_bytes = block_size * block_size * sizeof(float);

    // Initial total matrix
    const int n = 4;
    const int matrix_size = 1 << n;
    const int matrix_bytes = matrix_size * matrix_size * sizeof(float);

    const int block_count = matrix_size / block_size;

    printf("%d X %d cores\n", N, N);
    printf("core_blocks: %d X %d = %d bytes = 0x%x bytes\n", core_block_size, core_block_size, core_block_bytes, core_block_bytes);
    printf("blocks: %d X %d = %d bytes = 0x%x bytes\n", block_size, block_size, block_bytes, block_bytes);
    printf("full matrix: %d X %d = %d bytes = 0x%x bytes\n", matrix_size, matrix_size, matrix_bytes, matrix_bytes);

    // Prepare full matrix
    float* A = malloc(matrix_bytes);
    float* B = malloc(matrix_bytes);

    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            A[i * matrix_size + j] = (float)i;
            B[i * matrix_size + j] = (float)j;
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
            // When A is divided in block_size * block_size blocks
            // we want the block at block_Y, block_X
            // So that block's top-left element is
            // (i,j) = (block_Y * block_size , block_X * block_size)
            // Now loop i,j from 0 to block_size and use
            // A[ (block_Y * block_size + j) * matrix_size + (block_X * block_size + j) ]
            //
            // Within this block, we want to partition into 16 * 16 smaller blocks

            for (int i = 0; i < block_size; i++) {
                for (int j = 0; j < block_size; j++) {
                    float element_A = A[ (block_Y * block_size + i) * matrix_size + (block_X * block_size + j) ];
                    float element_B = B[ (block_X * block_size + i) * matrix_size + (block_Y * block_size + j) ];
                    // i,j are coordinates within the block
                    int X = j / core_block_size;
                    int Y = i / core_block_size;
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

    for (int p = 0; p < N * N; p++) {
        printf("Stream_A[%d] = \n", p);
        for (int i = 0; i < cur_index_A[p]; i++)
            printf("%5.0f ", stream_A[p][i]);
        printf("\nStream_B[%d] = \n", p);
        for (int i = 0; i < cur_index_B[p]; i++)
            printf("%5.0f ", stream_B[p][i]);
        printf("\n------------\n");
    }
    exit(-1);

    // Initialize the BSP system
    bsp_init("e_cannon.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    ebsp_spmd();

//    int packets = 0;
//    int accum_bytes = 0;
//    int status = 0;
//
//    ebsp_qsize(&packets, &accum_bytes);
//    tagsize = ebsp_get_tagsize();
//
//    float * matrix_block_C = matrix_block_A;
//    for (int i = 0; i < packets; i++)
//    {
//        ebsp_get_tag(&status, &tag);
//
//        int X = tag / N;
//        int Y = tag % N;
//        ebsp_move(matrix_block_C, block_size * block_size * sizeof(float));
//
//        int cur_index = 0;
//        for (int i = Y * block_size; i < (Y + 1) * block_size; i++) {
//            for (int j = X * block_size; j < (X + 1) * block_size; j++) {
//                C[i * matrix_size + j] = matrix_block_C[cur_index++];
//            }
//        }
//    }
//
//    free(matrix_block_A);
//    free(matrix_block_B);

    bsp_end();

    free(A);
    free(B);
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

