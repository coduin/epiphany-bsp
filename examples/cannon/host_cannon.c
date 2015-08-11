#include <stdlib.h>

int main()
{
    // we partition A, B in 4x4 blocks,
    // we then recursively? make them smaller, until
    // they fit on TLS

    // OR we say we store A as (say 2x2 large enough)
    //
    // 1  2  5  6
    // 3  4  7  8
    // 9  10 13 14
    // 11 12 15 16
    //
    // we define:
    //
    // ---------------------
    // |    |              |
    // |    | m            | ^
    // |-----              | |
    // |  m                | | n
    // |                   | |
    // |                   | v
    // |                   |
    // ---------------------
    //        <--->
    //          n
    //
    // so the 'small enough' submatrices are of size sxs

    const int procs = 16;
    const int m = 16;
    const int words_per_proc = m * m;

    // we have two matrices in main memory
    // A, B
    float* A = malloc(n * n * sizeof(float));
    float* B = malloc(n * n * sizeof(float));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            A[i * n + j] = (float)(rand() % 10);
            B[i * n + j] = (float)(rand() % 10);
        }
    }
    
    // we open *streams*
    bsp_stream_t streams[NPROCS];
    
    for (int s = 0; s < NPROCS; ++s) {
        streams[s] = ebsp_stream_open(s);
    }


}
