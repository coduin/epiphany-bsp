#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>

void print_matrix(float* A, int matrix_size);

int main(int argc, char **argv)
{
    // N * N cores
    const int N = 4;

    // block_size * block_size elements per matrix per core
    const int block_size = 32; // max 32
    const int block_bytes = block_size * block_size * sizeof(float);

    // Total matrix size
    const int matrix_size = N * block_size;
    const int matrix_bytes = matrix_size * matrix_size * sizeof(float);

    printf("%d X %d cores\n", N, N);
    printf("%d X %d matrix = %d bytes = 0x%x bytes\n", matrix_size, matrix_size, matrix_bytes, matrix_bytes);
    printf("%d X %d blocks per core = %d bytes = 0x%x bytes\n", block_size, block_size, block_bytes, block_bytes);

    float* A = malloc(matrix_bytes);
    float* B = malloc(matrix_bytes);
    float* C = malloc(matrix_bytes);

    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            A[i * matrix_size + j] = (float)i;
            B[i * matrix_size + j] = (float)j;
        }
    }

    printf("Matrix A = \n");
    print_matrix(A, matrix_size);
    printf("Matrix B = \n");
    print_matrix(B, matrix_size);

    // Initialize the BSP system
    bsp_init("e_cannon.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    int tagsize = 4;
    int tag = 0;
    ebsp_set_tagsize(&tagsize);

    // Partition

    float* matrix_block_A = malloc(block_bytes);
    float* matrix_block_B = malloc(block_bytes);

    for (int X = 0; X < N; X++) {
        for (int Y = 0; Y < N; Y++) {
            // Copy block X,Y to matrix_block
            int cur_index = 0;
            for (int i = Y * block_size; i < (Y + 1) * block_size; i++) {
                for (int j = X * block_size; j < (X + 1) * block_size; j++) {
                    matrix_block_A[cur_index] = A[i * matrix_size + j];
                    matrix_block_B[cur_index] = B[i * matrix_size + j];
                    cur_index++;
                }
            }

            //printf("Block A (X,Y) = (%d,%d):\n", X, Y);
            //print_matrix(matrix_block_A, block_size);
            //printf("Block B (X,Y) = (%d,%d):\n", X, Y);
            //print_matrix(matrix_block_B, block_size);

            // Target processor: P(i,j) gets A(i,i+j) and B(i+j,j)
            //
            // A(X,Y) goes to P(X,Y-X)
            // B(X,Y) goes to P(X-Y,Y)
            //
            // P(i,j) has PID 4*i + j

            int pidA = 4 * X + ((Y - X + N) % N);
            int pidB = 4 * ((X - Y + N) % N) + Y;

            tag = 1;
            ebsp_send_down(pidA, &tag, matrix_block_A, block_bytes);
            tag = 2;
            ebsp_send_down(pidB, &tag, matrix_block_B, block_bytes);
        }
    }

    ebsp_spmd();

    int packets = 0;
    int accum_bytes = 0;
    int status = 0;

    ebsp_qsize(&packets, &accum_bytes);
    tagsize = ebsp_get_tagsize();

    float * matrix_block_C = matrix_block_A;
    for (int i = 0; i < packets; i++)
    {
        ebsp_get_tag(&status, &tag);

        int X = tag / N;
        int Y = tag % N;
        ebsp_move(matrix_block_C, block_size * block_size * sizeof(float));

        int cur_index = 0;
        for (int i = Y * block_size; i < (Y + 1) * block_size; i++) {
            for (int j = X * block_size; j < (X + 1) * block_size; j++) {
                C[i * matrix_size + j] = matrix_block_C[cur_index++];
            }
        }
    }

    free(matrix_block_A);
    free(matrix_block_B);

    bsp_end();

    // Print results

    //printf("C = \n");
    //print_matrix(C, matrix_size);

    free(A);
    free(B);
    free(C);
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

