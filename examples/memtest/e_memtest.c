#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    char* a = (void*)0x4000;
    (*a) = 0;

    bsp_push_reg(a, -1);
    bsp_sync();

    if(p == 0) {
        (*a) = 'y';
        bsp_hpput(3, a, a, 0, 1);
    }

    bsp_end();

    return 0;
}
