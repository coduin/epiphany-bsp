#include <e_bsp.h>
#include "e-lib.h"

#include "common.h"

int M = 0;
int N = 0;
int s = 0;
int t = 0;
int dim = 0;
int entries_per_row = 0;

// "local to global" index
int ltg(int* i, int* j, int l)
{
    (*i) = s + (l % (dim / N)) * N;
    (*j) = t + (l % (dim / M)) * M;
}

// "global to local" index
inline int gtl(int i, int j)
{
    // assume correct processor
    return (i / (dim /  N)) * (dim / N) + (j / (dim / M));
}

inline float a(int i, int j) {
    return *((float*)LOC_MATRIX + gtl(i, j));
}

int main()
{
    int i, j, k;
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    M = (*(char*)LOC_M);
    N = (*(char*)LOC_N);
    dim = (*(char*)LOC_DIM);
    entries_per_row = dim / M;

    s = p / M;
    t = p % M;

    /* for (k = 0; k < dim; ++k) {
        // stage k
        if (k % M == t) {
            int max_value = -1;
            int rs = -1;
            for (i = k; i < dim; ++i) {
                if (i % N == s) {
                    int val = abs(a(i,k));
                    if (val > max_value) {
                        max_value = val;
                        rs = i;
                    }
                }
            }
        }
    } */

    //bsp_sync();

    int* result = (int*)LOC_RESULT;
    (*result) = s;
    (*(result + 1)) = t;
    
    bsp_end();

    return 0;
}
