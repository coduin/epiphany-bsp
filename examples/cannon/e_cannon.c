#include <e_bsp.h>
#include <stdint.h>
#include "common.h"

void get_initial_data(void* A, void* B);

void get_matrix_size(int* n);

void matrix_multiply_add(float* A, float* B, float* C);

void ebsp_set_head(int, int);

void ebsp_aligned_transfer(void* dest, const void* source, int nbytes)
{
    long long* dst = (long long*)dest;
    const long long* src = (const long long*)source;
    int count = nbytes >> 3;
    while(count--)
        *dst++ = *src++;
}

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

    a_data[0] = ebsp_malloc(CORE_BLOCK_BYTES);
    a_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[0] = ebsp_malloc(CORE_BLOCK_BYTES);
    b_data[1] = ebsp_malloc(CORE_BLOCK_BYTES);
    c_data    = ebsp_malloc(CORE_BLOCK_BYTES);

    bsp_push_reg(a_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(a_data[1], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[0], CORE_BLOCK_BYTES);
    bsp_sync();
    bsp_push_reg(b_data[1], CORE_BLOCK_BYTES);
    bsp_sync();

    int n = 0;
    get_matrix_size(&n);
    n /= BLOCK_SIZE;

    ebsp_raw_time();
    
    // Loop over the blocks (chunks)
    for (int cur_block = 0; cur_block < n * n * n; cur_block++)
    {
        if (cur_block != 0) {
            if (cur_block % (n * n) == 0) {
                ebsp_set_head(1, // stream id
                        -(n * n)); // relative chunk count
            } else if (cur_block % n == 0) {
                ebsp_set_head(0, // stream id
                        -n); // relative chunk count
            }
        }

        // Set C to zero
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++)
            c_data[i] = 0;

        // Obtain A, B
        get_next_chunk(a_data[0], // address
           0, // stream id
           0);// unbuffered mode
        get_next_chunk(b_data[0], // address
                1, // stream id
                0);// unbuffered mode

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
            bsp_hpput(a_neighbor, &a_data[cur], &a_data[cur_buffer], 0, CORE_BLOCK_BYTES);
            bsp_hpput(b_neighbor, &b_data[cur], &b_data[cur_buffer], 0, CORE_BLOCK_BYTES);

            // Perform C += A * B
            matrix_multiply_add(a_data[cur], b_data[cur], c_data);

            if (i == N - 1)
                break;

            // Switch buffers
            cur_buffer = (cur_buffer + 1) % 2;
            cur = (cur + 1) % 2;

            bsp_sync();
        }
        // TODO: Send result of C upwards
    }
    unsigned time = ebsp_raw_time();

    ebsp_message("cycles = %u, %f ms", time, time/600000.0f);

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
