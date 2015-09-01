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

    int a_neighbor = ((si + 1) % N) * N + sj;
    int b_neighbor = si * N + ((sj + 1) % N);

    // 5 buffers
    float* a_data[2];
    float* b_data[2];
    float* c_data;
    // neighbor buffer locations
    void* neighbor_a_data[2];
    void* neighbor_b_data[2];


    // Allocate local buffers
    a_data[0] = 0; //ebsp_malloc(CORE_BLOCK_BYTES);
    a_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[0] = 0; //ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    //TODO
    c_data = ebsp_malloc(CORE_BLOCK_BYTES);
    //c_data    = ebsp_open_out_stream(CORE_BLOCK_BYTES, 0);

    ebsp_message("%i", __LINE__);
    // Let ebsp malloc initial chunk
    ebsp_get_next_chunk((void**)&a_data[0], // address
            0, // stream id
            0);// single buffered mode
    ebsp_get_next_chunk((void**)&b_data[0], // address
            1, // stream id
            0);// single buffered mode

    ebsp_message("%i", __LINE__);
    // Register their locations
    bsp_push_reg(a_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(a_data[1], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[1], CORE_BLOCK_BYTES);
    bsp_sync();
    ebsp_message("%i", __LINE__);
   
    // Obtain neighbor locations
    neighbor_a_data[0] = ebsp_get_raw_address(a_neighbor, a_data[0]);
    neighbor_a_data[1] = ebsp_get_raw_address(a_neighbor, a_data[1]);
    neighbor_b_data[0] = ebsp_get_raw_address(b_neighbor, b_data[0]);
    neighbor_b_data[1] = ebsp_get_raw_address(b_neighbor, b_data[1]);
    ebsp_message("%i", __LINE__);

    int n = 0;
    get_matrix_size(&n);
    n /= BLOCK_SIZE;

    ebsp_dma_handle dma_handle_a;
    ebsp_dma_handle dma_handle_b;

    ebsp_host_sync();
    ebsp_message("%i", __LINE__);

    ebsp_raw_time();
    
    // Loop over the blocks (chunks)
    for (int cur_block = 0; cur_block < n * n * n; cur_block++)
    {
        if (cur_block != 0) {
            //TODO: add sizeof(float)
            //if (cur_block % (n * n) == 0) {
            //    ebsp_move_cursor(1, // stream id
            //            -(n * n)); // relative chunk count
            //} else if (cur_block % n == 0) {
            //    ebsp_move_cursor(0, // stream id
            //            -n); // relative chunk count
            //    // Send result of C upwards
            //    //TODO
            //    //ebsp_send_out_chunk(c_data);
            //}
        }

        // Set C to zero
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
            c_data[i] = 0;

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

        // Multiply this block, by looping over the core blocks
        for (int i = 0; i < N; i++)
        {
            // Target processor: P(i,j) gets A(i,i+j) and B(i+j,j)
            //
            // A(X,Y) goes to P(X,Y-X)
            // B(X,Y) goes to P(X-Y,Y)
            //
            // P(i,j) has PID 4*i + j

            // Send A,B to next core's "buffer"
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

    ebsp_message("cycles = %u, %f ms", time, time/600000.0f);

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
    for (int i = 0; i < BLOCK_SIZE; i++)
        for (int j = 0; j < BLOCK_SIZE; j++)
            for (int k = 0; k < BLOCK_SIZE; k++)
                C[i * BLOCK_SIZE + j] += A[i * BLOCK_SIZE + k] * B[k * BLOCK_SIZE + j];
}
