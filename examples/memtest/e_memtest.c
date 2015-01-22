#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    e_barrier_t sync_bar[_NPROCS];
    e_barrier_t* sync_bar_tgt[_NPROCS];
    e_barrier_init(sync_bar, sync_bar_tgt);
    e_barrier(sync_bar, sync_bar_tgt);
    e_barrier_init(sync_bar, sync_bar_tgt);

    bsp_sync();

    char* a = (void*)0x4000;
    (*a) = 0;
    char* b = (void*)0x4050;
    bsp_push_reg(a, -1);
    bsp_sync();
    bsp_push_reg(b, -1);
    bsp_sync();

    if(p == 0) {
        (*a) = 'y';
        bsp_hpput(3, a, a, 0, 1);
    }

    bsp_end();

    return 0;
}
