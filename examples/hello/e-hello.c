#include "include/bsp.h"

int main()
{
    int n = bsp_nprocs(); 
    int p = bsp_pid();

    return 0;
}
