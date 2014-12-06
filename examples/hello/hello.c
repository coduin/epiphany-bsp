#include <bsp_host.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // initialize the BSP system
    bsp_init("e-hello.srec", argc, argv);

    // show the number of processors available
    printf("bsp_nprocs(): %i\n", bsp_nprocs());

    // initialize the epiphany system, and load the e-program
    bsp_begin(bsp_nprocs());

    // run the SPMD on the e-cores
    spmd_epiphany();

    // finalize
    bsp_end();

    // read stuff??
    // ...
    // for now this is hardcoded in host lib

    return 0;
}
