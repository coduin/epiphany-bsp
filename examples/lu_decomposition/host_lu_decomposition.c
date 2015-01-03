#include <host_bsp.h>

#include <stdlib.h>
#include <stdio.h>

// distribution function, returns proc id
int phi(i, j, N, M)
{
    
}

// Functions to convert between global and local coordinates
// Local to global
int ltg(int i, int j, int& l)
{

}

// Global to local
int ltg(int& i, int& j, int l)
{

}

int main(int argc, char **argv)
{
    // for our example we decompose the (k x k) Hilbert matrix
    int k = 10;
    float* hilb = calloc(sizeof(float) * k * k);

    int i = 0; 
    int j = 0;
    for(i = 1; i <= k; ++i) {
        for(j = 1; j <= k; ++j) {
            hilb[k*(i-1) + (j-1)] = 1.0/(i+j-1) 
        }
    }

    bsp_init("bin/e_lu_decomposition.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // distribute the matrix
    int N = 0;
    int M = 0;
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

    int pcol, prow;
    for(i = 1; i <= k; ++i) {
        for(j = 1; j <= k; ++j) {
            int pid = phi(i, j, N, M);
            co_write(pid,
                    &hilb[k*(i-1) + (j-1)],
                    0x4000, sizeof(float));
        }
    }

    spmd_epiphany();

    // read L and U
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        int p;
        int pcol, prow;
        _get_p_coords(pid, &prow, &pcol);
        e_read(&(_get_state()->dev),
                prow, pcol,
                (off_t)0x7050, &msg, 1);
        e_read(&(_get_state()->dev),
                prow, pcol,
                (off_t)0x7100, &p, sizeof(int));
        printf("%i: %c\n", p, msg);
    }

    // show decomposition

    // finalize
    bsp_end();

    return 0;
}
