#include <bsp_host.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    bsp_init("e-hello", argc, argv);
    printf("bsp_nprocs(): %i\n", bsp_nprocs());
    //bsp_begin(bsp_nprocs());

    return 0;
}
