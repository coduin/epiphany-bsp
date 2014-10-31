#include "include/bsp_host.h"

int main(int argc, char **argv)
{
    bsp_init("e-hello", argc, arv);

    bsp_begin(bsp_nprocs());
    bsp_end();
}
