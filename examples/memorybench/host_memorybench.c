#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char **argv)
{
    if(!bsp_init("e_memorybench.srec", argc, argv)) {
        fprintf(stderr, "[BSPBENCH] bsp_init() failed\n");
        return -1;
    }
    if (!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[BSPBENCH] bsp_begin() failed\n");
    }

    ebsp_spmd();
    bsp_end();
    
    return 0;
}
