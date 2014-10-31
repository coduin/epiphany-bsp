#include "bsp_host.h"

#include <e-hal.h>

typedef struct _bsp_state_t
{
    int n_procs;

    // Maintain stack for every processor?
    int* memory;
} bsp_state_t;

// Global state
bsp_state_t state;

void bsp_init(const char* e_name,
        int argc,
        char **argv)
{
    state.n_procs = 0;
}

void bsp_begin(int nprocs)
{
    return;
}

void bsp_end()
{
    return;
}

int bsp_nprocs()
{
    return state.n_procs;
}
