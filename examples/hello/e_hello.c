#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    // set char */
    char* a = (void*)0x7050;
    (*a) = hmsg[p];
    int* po = (void*)0x7100;
    int tmp=CORE_ID;
    (*po) = tmp;

    return 0;
}
