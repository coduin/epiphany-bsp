#include <host_bsp.h>

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

// information on matrix and procs
char N = -1;
char M = -1;
int k = -1;

// "local to global" index
int ltg(int& i, int& j, int l, int s, int t)
{
    rows_per_proc = k / N;
    cols_per_proc = k / M;
    i = (l / cols_per_proc) * rows_per_proc + rows_per_proc % pid + s; // FIXME
    j = (l / cols_per_proc) * rows_per_proc + rows_per_proc % pid + t; // FIXME
}

// "global to local" index
int gtl(int i, int j, int& l)
{
    l = i % M + 3; // FIXME
}

int main(int argc, char **argv)
{
    // for our example we decompose the (k x k) Hilbert matrix
    k = 10;

    // allocate and zero-initialize matrix
    float* hilb = calloc(sizeof(float) * k * k);

    // construct the matrix H, which is defined as H_ij = 1/(i+j-1)
    int i = 0; 
    int j = 0;
    for(i = 1; i <= k; ++i) {
        for(j = 1; j <= k; ++j) {
            hilb[k*(i-1) + (j-1)] = 1.0/(i+j-1) 
        }
    }

    // initialize the BSP system
    bsp_init("bin/e_lu_decomposition.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // distribute the matrix
    switch(bsp_nprocs()) {
        case 16:
            N = 4;
            M = 4;
            break;

        case 64:
            N = 8;
            M = 8;
            break;

        default:
            fprintf(stderr, "Unsupported processor count, please add values\
                    for N and M in the host program.");
            break;
    }

    // Write M to every processor such that they can figure out the (s,t) pair
    for(i = 0; i < bsp_nprocs(); ++i) {
        co_write(i, M, LOC_M, sizeof(char));
    }

    int pcol, prow;
    int l = 0;
    for(i = 1; i <= k; ++i) {
        for(j = 1; j <= k; ++j) {
            int pid = phi(i, j);
            co_write(pid,
                    &hilb[k*(i-1) + (j-1)],
                    LOC_MATRIX + sizeof(float) * l,
                    sizeof(float));
        }
    }

    spmd_epiphany();

    // read L and U
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        //co_read
        //printf
    }

    // show decomposition

    // finalize
    bsp_end();

    return 0;
}
