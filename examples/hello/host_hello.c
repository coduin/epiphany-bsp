#include <host_bsp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // initialize the BSP system
    if(!bsp_init("bin/e_hello.srec", argc, argv)) {
        fprintf(stderr, "[HELLO] bsp_init() failed\n");
        return -1;
    }

    // show the number of processors available
    printf("bsp_nprocs(): %i\n", bsp_nprocs());

    // initialize the epiphany system, and load the e-program
    if(!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[HELLO] bsp_begin() failed\n");
        return -1;
    }

    // run the SPMD on the e-cores
    ebsp_spmd();

    // read messages
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        int p;
        co_read(pid, (off_t)0x6000, &msg, sizeof(char));
        co_read(pid, (off_t)0x6004, &p, sizeof(int));
        printf("%i: %c\n", p, msg);
    }

    // finalize
    bsp_end();

    return 0;
}
