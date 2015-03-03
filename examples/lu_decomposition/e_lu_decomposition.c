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

#include <math.h>
#include "common.h"

int M = 0;
int N = 0;
int dim = 0;
int s = 0;
int t = 0;
int entries_per_col = 0;

int proc_id(int s, int t)
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
int gtl(int i, int j)
{
    // here we assume correct processor
    return (i / M) * (dim / M) + (j / M);
}

float* a(int i, int j) {
    return (float*)LOC_MATRIX + gtl(i, j);
}

int main()
{
    bsp_begin();

    ebsp_message("(-5)");

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    ebsp_message("(-5a)");

    ebsp_message("test");

    M = (*(int*)LOC_M);
    N = (*(int*)LOC_N);
    dim = (*(int*)LOC_DIM);

    bsp_sync();

    ebsp_message("M, N, dim: %i, %i, %i", M, N, dim);

    entries_per_col = dim / M;

    ebsp_message("test3");

    s = p / M;
    t = p % M;

    // register variable to store r and a_rk
    // need arrays equal to number of procs in our proc column
    bsp_push_reg((void*)LOC_RS, sizeof(int) * N);
    ebsp_message("(-5c)");
    bsp_sync();
    ebsp_message("(-4)");

    bsp_push_reg((void*)LOC_ARK, sizeof(float) * N);
    bsp_sync();
    ebsp_message("(-3)");

    bsp_push_reg((void*)LOC_R, sizeof(int));
    bsp_sync();
    ebsp_message("(-2)");

    // FIXME: PI is actually distributed as well.
    bsp_push_reg((void*)LOC_PI_IN, sizeof(int));
    bsp_sync();
    ebsp_message("(-1)");

    // also initialize pi as identity
    if (t == 0)
        for (int i = 0; i < entries_per_col; ++i)
            *((int*)LOC_PI + i) = i;

    for (int k = 0; k < dim; ++k) {

        //----------------------
        // STAGE 1: Pivot search
        //----------------------
        if (k % M == t) {
            // COMPUTE PIVOT IN COLUMN K
            int rs = -1;
            float a_rk = -1.0;

            ebsp_message("(0)");

            int start_i = (k / N) * N + s;
            if (s % N < k % N)
                start_i += N;

            for (int i = start_i; i < dim; i += N) {
                float a_ik = fabsf(*a(i, k));
                if (a_ik > a_rk) {
                    a_rk = a_ik;
                    rs = i;
                }
            }

            // HORIZONTAL COMMUNICATION
            for (int j = 0; j < N; ++j) {
                // put r_s in P(*,t)
                bsp_hpput(proc_id(j, t),
                         &rs, (void*)LOC_RS,
                         s * sizeof(int), sizeof(int));

                // put a_(r_s, k) in P(*,t)
                bsp_hpput(proc_id(j, t),
                         &rs, (void*)LOC_ARK,
                         s * sizeof(float), sizeof(float));
            }

            ebsp_message("(0) + (1)");
            bsp_sync(); // (0) + (1)

            a_rk = -1.0;
            for (int j = 0; j < N; ++j) {
                float val = fabsf(*(((float*)LOC_ARK + j)));
                if (val > a_rk) {
                    a_rk = val;
                    rs = *((int*)LOC_RS + j);
                }
            }

            // put r in P(s, *)
            for(int j = 0; j < M; ++j) {
                bsp_hpput(proc_id(s, j),
                        &rs, (void*)LOC_R,
                        0, sizeof(int));
            }

            ebsp_message("(2) + (3)");
            bsp_sync(); // (2) + (3)
        }
        else {
            ebsp_message("(!0)");
            ebsp_message("(0) + (1)");
            bsp_sync(); // (0) + (1)
            ebsp_message("(2) + (3)");
            bsp_sync(); // (2) + (3)
        }

        // ----------------------------
        // STAGE 2: Index and row swaps
        // ----------------------------
        int r = *((int*)LOC_R);

        if (k % N == s && t == 0) {
            bsp_hpput(proc_id(r % N, 0),
                    ((int*)LOC_PI + (k / N)), (void*)LOC_PI_IN,
                    0, sizeof(int));
        }

        if (r % N == s && t == 0) {
            // here offset is set to one in case k % N == r % N
            bsp_hpput(proc_id(k % N, 0),
                    ((int*)LOC_PI + (r / N)), (void*)LOC_PI_IN,
                    sizeof(int), sizeof(int));
        }

        bsp_sync(); // (4)

        if (k % N == s && t == 0)
            *((int*)LOC_PI + (k / N)) = *((int*)LOC_PI_IN + 1);

        if (r % N == s && t == 0)
            *((int*)LOC_PI + (r / N)) = *((int*)LOC_PI_IN);

        if (k % N == s) { // need to swap rows with row r
            for (int j = t; j < dim; j += M) {
                 bsp_hpput(proc_id(r % N, t),
                        a(k, j), (void*)LOC_ROW_IN,
                        sizeof(float) * (j - t) / M, sizeof(float));
            }
        }

        if (r % N == s) { // need to swap rows with row k
            for (int j = t; j < dim; j += M) {
                 bsp_hpput(proc_id(k % N, t),
                        a(r, j), (void*)LOC_ROW_IN,
                        sizeof(float) * (j - t) / M, sizeof(float));
            }
        }
 
        bsp_sync(); // (5) + (6)

        if (k % N == s) {
            for (int j = t; j < dim; j += M) {
                *a(k, j) = *((float*)LOC_ROW_IN + (j - t)/M);
            }
        }
        if (r % N == s) {
            for (int j = t; j < dim; j += M) {
                *a(r, j) = *((float*)LOC_ROW_IN + (j - t)/M);
            }
        }

        bsp_sync(); // (7)

        // ----------------------
        // STAGE 3: Matrix update
        // ----------------------
        if (k % N == s && k % M == t) {
            // put a_kk in P(*, t)
            for (int j = 0; j < N; j += M) {
                 bsp_hpput(proc_id(j, t),
                        a(k, k), (void*)LOC_ROW_IN,
                        0, sizeof(float));
            }
        }

        bsp_sync(); // (8)

        int start_idx = (k / N) * N + s;
        if (s % N <= k % N)
            start_idx += N;

        if (k % M == t) {
            for (int i = start_idx; i < dim; i += N) {
                *a(i, k) = *a(i, k) / (*((int*)LOC_ROW_IN));
            }
        }

        // HORIZONTAL COMMUNICATION
        if (k % M == t) {
            // put a_ik in P(s, *)
            for (int i = start_idx; i < dim; i += N) {
                for (int j = 0; j < M; ++j) {
                    bsp_hpput(proc_id(s, j),
                            a(i, k), (void*)LOC_COL_IN,
                            sizeof(float) * i, sizeof(float));
                }
            }
        }

        // VERTICAL COMMUNICATION
        if (k % N == s) {
            // put a_ki in P(*, t)
            for (int j = start_idx; j < dim; j += M) {
                for (int sj = 0; sj < N; ++sj) {
                    bsp_hpput(proc_id(sj, t),
                            a(k, j), (void*)LOC_ROW_IN,
                            sizeof(float) * j, sizeof(float));
                }
            }
        }

        bsp_sync(); // (9) + (10)

        for (int i = start_idx; i < dim; i += N) {
            for (int j = start_idx; j < dim; j += M) {
                int a_ik = *((float*)LOC_COL_IN + i);
                int a_kj = *((float*)LOC_ROW_IN + j);
                *a(i, j) = *a(i, j) - a_ik * a_kj;
            }
        }
    }

    bsp_end(); // (11)

    return 0;
}
