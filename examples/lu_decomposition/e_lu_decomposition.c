#include <e_bsp.h>
#include "e-lib.h"

#include "common.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    char* M = LOC_M;

    int s = p / (*M);
    int t = p % (*M);

    bsp_sync();

    int* result = LOC_RESULT;
    (*result) = s;
    (*result + 1) = t;

    return 0;
}
