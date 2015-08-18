#include <e_bsp.h>
#include <stdint.h>

void get_initial_data(void* A, void* B);

void matrix_multiply(float* A, float* B, float* C, int block_size);

void fast_transfer(void* dest, const void* source, int nbytes)
{
    long long* dst = (long long*)dest;
    const long long* src = (const long long*)source;
    int count = nbytes >> 3;
    while(count--)
        *dst++ = *src++;
}

int main()
{
    // N * N cores
    const int N = 4;

    // block_size * block_size elements per matrix per core
    const int block_size = 32; // max 32

    bsp_begin();
    int p = bsp_pid();

    int Pi = p / N;
    int Pj = p % N;

    const int total_matrix_bytes = block_size * block_size * sizeof(float);

    float* A = ebsp_malloc(total_matrix_bytes);
    float* B = ebsp_malloc(total_matrix_bytes);
    float* C = ebsp_malloc(total_matrix_bytes);
    float* buffer = ebsp_malloc(total_matrix_bytes);

    // Set C to zero
    for (int i = 0; i < block_size * block_size; i++)
        C[i] = 0;

    // Obtain A, B
    get_initial_data(A, B);

    bsp_push_reg(buffer, total_matrix_bytes);
    bsp_sync();

    // Multiply
    ebsp_raw_time();
    for (int i = 0; i < N; i++)
    {
        // Perform C += A * B
        matrix_multiply(A, B, C, block_size);

        if (i == N - 1)
            break;

        bsp_sync();

        // Target processor: P(i,j) gets A(i,i+j) and B(i+j,j)
        //
        // A(X,Y) goes to P(X,Y-X)
        // B(X,Y) goes to P(X-Y,Y)
        //
        // P(i,j) has PID 4*i + j

        // Send A to next core

        bsp_hpput(((Pi + 1) % N) * N + Pj, A, buffer, 0, total_matrix_bytes);
        bsp_sync();

        fast_transfer(A, buffer, total_matrix_bytes);
        bsp_sync();

        // Send B to next core

        bsp_hpput(Pi * N + ((Pj + 1) % N), B, buffer, 0, total_matrix_bytes);
        bsp_sync();

        fast_transfer(A, buffer, total_matrix_bytes);
    }
    unsigned time = ebsp_raw_time();

    ebsp_message("cycles = %u, %f ms", time, time/600000.0f);

    int tag = p;
    ebsp_send_up(&tag, C, total_matrix_bytes);

    bsp_end();
}

void get_initial_data(void* A, void* B)
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
            bsp_move(A, status);
        else if (tag == 2)
            bsp_move(B, status);
        else
            ebsp_message("Received unknown tag %d", tag);
    }
}

void matrix_multiply(float* A, float* B, float* C, int block_size)
{
    for (int i = 0; i < block_size; i++)
        for (int j = 0; j < block_size; j++)
            for (int k = 0; k < block_size; k++)
                C[i * block_size + j] += A[i * block_size + k] * B[k * block_size + j];
}
