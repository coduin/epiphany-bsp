/*
File: e_lu_decomposition.h

This file is part of the Epiphany BSP library.

Copyright (C) 2014 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/

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

inline float* a(int i, int j) {
    return (float*)LOC_MATRIX + gtl(i, j);
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

    // register variable to store r and a_rk
    // need arrays equal to number of procs in our proc column
    bsp_push_reg((void*)LOC_RS, sizeof(int) * N);
    bsp_sync();

    bsp_push_reg((void*)LOC_ARK, sizeof(float) * N);
    bsp_sync();

    bsp_push_reg((void*)LOC_R, sizeof(int));
    bsp_sync();

    // FIXME: PI is actually distributed as well.
    bsp_push_reg((void*)LOC_PI_IN, sizeof(int));
    bsp_sync();

    // also initialize pi as identity
    if (t == 0)
        for (i = 0; i < N; ++i)
            *((int*)LOC_PI + i) = i;

    for (k = 0; k < dim; ++k) {

        //----------------------
        // STAGE 1: Pivot search
        //----------------------
        if (k % M == 0) {
            int rs = -1;
            float a_rk = -1.0;
            for (i = k; i < dim; ++i) {
                float a_ik = abs(*a(i, k));
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
                bsp_hpput(proc_id(s, j),
                        &rs, (void*)LOC_R,
                        0, sizeof(int));
            }

            bsp_sync(); // (2) + (3)
        }
        else {
            bsp_sync(); // (0) + (1)
            bsp_sync(); // (2) + (3)
        }

        // ----------------------------
        // STAGE 2: Index and row swaps
        // ----------------------------
        int r = *((int*)LOC_R);
        if (k % N == s && t == 0) {
            bsp_hpput(proc_id(r % N, 0),
                    ((int*)LOC_PI + k), (void*)LOC_PI_IN,
                    0, sizeof(int));
        }
        if (r % N == s && t == 0) {
            bsp_hpput(proc_id(k % N, 0),
                    ((int*)LOC_PI + r), (void*)LOC_PI_IN,
                    sizeof(int), sizeof(int));
        }
        bsp_sync(); // (4)

        if (k % N == s && t == 0)
            *((int*)LOC_PI + k) = *((int*)LOC_PI_IN + 1);
        if (r % N == s && t == 0)
            *((int*)LOC_PI + r) = *((int*)LOC_PI_IN);

        if (k % N == s) { // need to swap rows with row r
            for (j = t; j < dim; j += M) {
                 bsp_hpput(proc_id(r % N, t),
                        a(k, j), (void*)LOC_ROW_IN,
                        sizeof(float) * (j - t) / M, sizeof(float));
            }
        }

        if (r % N == s) { // need to swap rows with row r
            for (j = t; j < dim; j += M) {
                 bsp_hpput(proc_id(k % N, t),
                        a(r, j), (void*)LOC_ROW_IN,
                        sizeof(float) * (j - t) / M, sizeof(float));
            }
        }
 
        bsp_sync(); // (5) + (6)

        if (k % N == s) {
            for (j = t; j < dim; j += M) {
                (*a(k, j)) = *((float*)LOC_ROW_IN + (j - t)/M);
            }
        }
        if (r % N == s) {
            for (j = t; j < dim; j += M) {
                (*a(r, j)) = *((float*)LOC_ROW_IN + (j - t)/M);
            }
        }

        bsp_sync(); // (7)

        // ----------------------
        // STAGE 3: Matrix update
        // ----------------------
        if (k % N == s && k % M == t) {
            // put a_kk in P(*, t)
            for (j = 0; j < N; j += M) {
                 bsp_hpput(proc_id(j, t),
                        a(k, k), (void*)LOC_ROW_IN,
                        0, sizeof(float));
            }
        }

        bsp_sync(); // (8)

        if (k % M == t) {
            for (i = k; i < n && i % N == s; ++i) {
                (*a(i, k)) = *a(i,k) / (*((int*)LOC_ROW_IN));
            }
        }

        if (k % M == t) {
            // put a_ik in P(s, *)
            for (i = k; i < n && i % N == s; ++i) {
                for (j = 0; j < M; ++j) {
                    bsp_hpput(proc_id(s, j),
                            a(i, k), (void*)LOC_COL_IN,
                            sizeof(float) * i, sizeof(float));
                }
            }
        }
        if (k % N == s) {
            // put a_ki in P(*, t)
            for (i = k; i < n && i % M == t; ++i) {
                for (j = 0; j < N; ++j) {
                    bsp_hpput(proc_id(j, t),
                            a(k, i), (void*)LOC_ROW_IN,
                            sizeof(float) * i, sizeof(float));
                }
            }
        }

        bsp_sync(); // (9) + (10)

        for (i = k; i < n && i % N == s; ++i) {
            for (j = k; j < n && j % M == t; ++j) {
                int a_ik = *((float*)LOC_COL_IN + i);
                int a_kj = *((float*)LOC_ROW_IN + j);
                (*a(i, j)) = *a(i, j) - a_ik * a_kj;
            }
        }
    }

    bsp_end(); // (11)

    return 0;
}
