#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    char* a = (void*)0x7050;
    (*a) = hmsg[p];
    
    bsp_push_reg(&regvar, sizeof(int));
    bsp_sync();
    int regvar=registermap[n*0+p];
    if(p == 4) {
        int tmp=999;
        //bsp_hpput(5, &tmp, &regvar, 0, sizeof(int));
    }
    bsp_sync();
    
    int* po = (void*)0x7100;
    (*po) = regvar;


    return 0;
}
