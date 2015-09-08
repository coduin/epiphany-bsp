#include <e_bsp.h>
#include <stdint.h>
#include "common.h"

void get_initial_data(void* A, void* B);

void get_matrix_size(int* n);

void matrix_multiply_add(float* A, float* B, float* C);

void ebsp_set_head(int, int);

int main()
{
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

    int matrix_size = 0;
    get_matrix_size(&matrix_size);
    int n = matrix_size / BLOCK_SIZE;
    //ebsp_message("n = %i", n);

    // Allocate local buffers
    a_data[0] = 0; //ebsp_malloc(CORE_BLOCK_BYTES);
    a_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[0] = 0; //ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    //TODO
    c_data = ebsp_malloc(CORE_BLOCK_BYTES);
    //c_data    = ebsp_open_out_stream(CORE_BLOCK_BYTES, 0);

    // Set C to zero
    for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
        c_data[i] = 0;

    // Let ebsp malloc initial chunk
    ebsp_get_next_chunk((void**)&a_data[0], // address
            0, // stream id
            0);// single buffered mode
    ebsp_get_next_chunk((void**)&b_data[0], // address
            1, // stream id
            0);// single buffered mode

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
    neighbor_a_data[0] = ebsp_get_raw_address(a_neighbor, a_data[0]);
    neighbor_a_data[1] = ebsp_get_raw_address(a_neighbor, a_data[1]);
    neighbor_b_data[0] = ebsp_get_raw_address(b_neighbor, b_data[0]);
    neighbor_b_data[1] = ebsp_get_raw_address(b_neighbor, b_data[1]);

    //ebsp_barrier();
    //ebsp_message("a_neighbor = %i ; b_neighbor = %i", a_neighbor, b_neighbor);
    //ebsp_barrier();
    //ebsp_message("c_data = %p", c_data);
    //ebsp_barrier();
    //ebsp_barrier();
    //ebsp_message("a_data[0] = %i %i %i \t a_data[1] = %i %i %i", (int)a_data[0][0], (int)a_data[0][1], (int)a_data[0][2],
    //        (int)a_data[1][0], (int)a_data[1][1], (int)a_data[1][2]);
    //ebsp_barrier();
    //ebsp_message("n_a_data[0] = %i %i %i \t n_a_data[1] = %i %i %i", (int)neighbor_a_data[0][0], (int)neighbor_a_data[0][1], (int)neighbor_a_data[0][2],
    //        (int)neighbor_a_data[1][0], (int)neighbor_a_data[1][1], (int)neighbor_a_data[1][2]);

    ebsp_dma_handle dma_handle_a;
    ebsp_dma_handle dma_handle_b;

    ebsp_host_sync();
    ebsp_barrier();

    ebsp_raw_time();
    
    // Loop over the blocks (chunks)
    // these are the *global blocks*
    for (int cur_block = 0; cur_block <= n * n * n; cur_block++)
    {
        if (cur_block != 0) {
            if (cur_block % (n * n) == 0) {
                ebsp_move_in_cursor(1, // stream id
                        -(n * n)); // relative chunk count
            }
            else if (cur_block % n == 0) {
                ebsp_move_in_cursor(0, // stream id
                        -n); // relative chunk count
            }
            if (cur_block % n == 0) {
                // Send result of C upwards
                //TODO
                //ebsp_send_out_chunk(c_data);
                // FIXME find more elegant way of accomplishing this.
                if (cur_block == n * n * n) {
                    ebsp_message("%i (%i, %i, %i, ..., %i)",
                            cur_block, (int)c_data[0],
                            (int)c_data[1],
                            (int)c_data[2],
                            (int)c_data[CORE_BLOCK_SIZE * CORE_BLOCK_SIZE - 1]);
                    ebsp_barrier();
                    break;
                }

                // Set C to zero
                for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
                    c_data[i] = 0;
            }
        }

        // Obtain A, B
        if (cur_block != 0) {
            ebsp_get_next_chunk((void**)&a_data[0], // address
                    0, // stream id
                    0);// single buffered mode
            ebsp_get_next_chunk((void**)&b_data[0], // address
                    1, // stream id
                    0);// single buffered mode
        }

        int cur = 0; // computation
        int cur_buffer = 1; // data transfer

        // Multiply this block, by looping over the *core blocks*
        for (int i = 0; i < N; i++)
        {
            // Target processor: P(i,j) gets A(i,i+j) and B(i+j,j)
            //
            // A(X,Y) goes to P(X,Y-X)
            // B(X,Y) goes to P(X-Y,Y)
            //
            // P(i,j) has PID 4*i + j
            if (i != N - 1) {
                ebsp_dma_push(&dma_handle_a, neighbor_a_data[cur_buffer], a_data[cur], CORE_BLOCK_BYTES);
                ebsp_dma_push(&dma_handle_b, neighbor_b_data[cur_buffer], b_data[cur], CORE_BLOCK_BYTES);
            }

            // Perform C += A * B
            matrix_multiply_add(a_data[cur], b_data[cur], c_data);

            if (i == N - 1)
                break;

            // Switch buffers
            cur_buffer = (cur_buffer + 1) % 2;
            cur = (cur + 1) % 2;

            ebsp_dma_wait(&dma_handle_a);
            ebsp_dma_wait(&dma_handle_b);
            ebsp_barrier();
        }
    }
    unsigned time = ebsp_raw_time();

    //ebsp_message("cycles = %u, %f ms", time, time/600000.0f);
    //ebsp_close_out_stream(c_data);

    bsp_end();
}

void get_matrix_size(int* n)
{
    int packets = 0;
    int accum_bytes = 0;
    int status = 0;
    int tag = 0;

    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++)
    {
        bsp_get_tag(&status, &tag);
        if (tag == 1)
            bsp_move(n, sizeof(int));
    }
}
 
// TODO: assembly
void matrix_multiply_add(float* A, float* B, float* C)
{
//            ebsp_message("a[0] = %i", (int)A[0]);
//            ebsp_message("b[0] = %i", (int)B[0]);
//            ebsp_message("c[0] = %i", (int)C[0]);


    for (int i = 0; i < CORE_BLOCK_SIZE; i++)
        for (int j = 0; j < CORE_BLOCK_SIZE; j++)
            for (int k = 0; k < CORE_BLOCK_SIZE; k++)
                C[i * CORE_BLOCK_SIZE + j] += A[i * CORE_BLOCK_SIZE + k] * B[k * CORE_BLOCK_SIZE + j];
}
