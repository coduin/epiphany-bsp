#include <host_bsp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // initialize the BSP system
    if(!bsp_init("bin/e_puttest.srec", argc, argv)) {
        fprintf(stderr, "[HELLO] bsp_init() failed\n");
        return -1;
    }

    printf("Welcome to puttest!\n");
    // show the number of processors available
    printf("bsp_nprocs(): %i\n", bsp_nprocs());

    // initialize the epiphany system, and load the e-program
    if(!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[HELLO] bsp_begin() failed\n");
        return -1;
    }

    // run the SPMD on the e-cores
    spmd_epiphany();

    // read messages
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        int p;
        int pcol, prow;
        _get_p_coords(pid, &prow, &pcol);
        e_read(&(_get_state()->dev), prow, pcol, (off_t)0x7050, &msg, 1);
        e_read(&(_get_state()->dev), prow, pcol, (off_t)0x7100, &p, sizeof(int));
        printf("%i: %c\n", p, msg);
    }

    // finalize
    bsp_end();

    return 0;
}
