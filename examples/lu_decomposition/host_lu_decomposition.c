#include <host_bsp.h>

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define DEBUG

// information on matrix and procs
char N = -1;
char M = -1;

// always choose dim 40 such that we dont have to worry about heterogeneous
// distributions
char dim = 40;

// "local to global" index
int ltg(int& i, int& j, int l, int s, int t)
{
    i = s + (l % (dim / N)) * N;
    j = t + (l % (dim / M)) * M;
}

// "global to local" index
int gtl(int i, int j, int& l, int& s, int& t)
{
    s = i % N;
    t = j % M;
    l = (i / (dim /  N)) * (dim / N) + (j / (dim / M));
}

int proc_id(int s, int t)
{
    return s * M + t;
}

int main(int argc, char **argv)
{
    // allocate and zero-initialize matrix
    float* hilb = calloc(sizeof(float) * dim * dim);

    // construct the matrix H, which is defined as H_ij = 1/(i+j-1)
    int i = 0; 
    int j = 0;
    for(i = 0; i < dim; ++i) {
        for(j = 0; j < dim; ++j) {
            hilb[dim*i + j] = 1.0/(i+j+1) // store zero based (i.e. a_00 top-left)
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

    // Write M, N and dim to every processor such that they can figure out 
    // the (s,t) pair, and gtl / ltg functions
    for(i = 0; i < bsp_nprocs(); ++i) {
        co_write(i, M, LOC_M, sizeof(char));
        co_write(i, N, LOC_N, sizeof(char));
        co_write(i, dim, LOC_DIM, sizeof(char));
    }

    int s = 0;
    int t = 0;
    int l = 0;
    for(i = 0; i < dim; ++i) {
        for(j = 0; j < dim; ++j) {
            gtl(i, j, l, s, t);
            co_write(proc_id(s, t),
                    &hilb[dim*i + j],
                    LOC_MATRIX + sizeof(float) * l,
                    sizeof(float));
        }
    }

    // test global to local and local to global function for random processor
#ifdef DEBUG
    s = 2;
    t = 3;
    for(l = 0; l < dim * dim; ++l) {
            ltg(i, j, l, s, t);
            float val;
            co_write(proc_id(s, t),
                    LOC_MATRIX + sizeof(float) * l,
                    &val,
                    sizeof(float));
            printf("(%i, %i) %f\n", i, j, val);
    }
#endif

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
