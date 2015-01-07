#include <e_bsp.h>
#include "e-lib.h"

#include "common.h"

int M = 0;
int N = 0;
int s = 0;
int t = 0;
int dim = 0;
int entries_per_row = 0;

inline int proc_id(int s, int t)
{
    return s * M + t;
}

// "local to global" index
int ltg(int* i, int* j, int l)
{
    (*i) = s + (l % (dim / N)) * N;
    (*j) = t + (l % (dim / M)) * M;
}

// "global to local" index
inline int gtl(int i, int j)
{
    // here we assume correct processor
    return (i / M) * (dim / M) + (j / M);
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

    // STAGE 1: Pivoting
    // register variable to store r and a_rk
    // need arrays equal to number of procs in our proc row
    bsp_push_reg((void*)LOC_RS, sizeof(int) * N);
    bsp_sync();

    bsp_push_reg((void*)LOC_ARK, sizeof(float) * N);
    bsp_sync();

    bsp_push_reg((void*)LOC_R, sizeof(int));
    bsp_sync();

    for (k = 0; k < dim; ++k) {
        if (k % M == 0) {
            int rs = -1;
            float a_rk = -1.0;
            for (i = k; i < dim; ++i) {
                float a_ik = abs(a(i,k));
                if (a_ik > a_rk) {
                    a_rk = a_ik;
                    rs = i;
                }
            }

            for (j = 0; j < N; ++j) {
                // put r_s in P(*,t)
                bsp_hpput(proc_id(j, t),
                         &rs, (void*)LOC_RS,
                         s * sizeof(int), sizeof(int));

                // put a_(r_s, k) in P(*,t)
                bsp_hpput(proc_id(j, t),
                         &rs, (void*)LOC_ARK,
                         s * sizeof(float), sizeof(float));
            }

            bsp_sync(); // (0) + (1)

            a_rk = -1.0;
            for (j = 0; j < N; ++j) {
                float val = abs(*(((float*)LOC_ARK + j)));
                if (val > a_rk) {
                    a_rk = val;
                    rs = *((int*)LOC_RS + j);
                }
            }

            // put r in P(s, *)
            for(j = 0; j < M; ++j) {
                bsp_hpput(proc(s, j),
                        &rs, (void*)LOC_R,
                        0, sizeof(int));
            }

            bsp_sync(); // (2) + (3)
        }
        else {
            bsp_sync(); // (0) + (1)
            bsp_sync(); // (2) + (3)
        }
    }


    int* result = (int*)LOC_RESULT;
    (*result) = s;
    (*(result + 1)) = t;
    
    bsp_end();

    return 0;
}
