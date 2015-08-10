#include <e_bsp.h>

int main()
{
    const int s = 4;
    const int n = 512;
    const int m = 16;

    bsp_begin();

    for (int k = 0; k < s; ++k) {
        // receive two matrix A[i,q] and B[q,j] that we multiply
        // these are in a stream
        // a chunk should be large enough 
        ebsp_next_chunk();
        for (int i = 0; i < n; ++i) {
            float aik = (float)ebsp_next_element();
            float bkj = (float)ebsp_next_element();
            cij += aik * bkj;
        }
    }

    bsp_end();
}
