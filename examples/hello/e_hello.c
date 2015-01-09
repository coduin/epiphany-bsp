#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    char* a = (void*)0x6000;
    (*a) = hmsg[p];
    int* po = (int*)0x6004;
    (*po) = p;

    bsp_end();

    return 0;
}
