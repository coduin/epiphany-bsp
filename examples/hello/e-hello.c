#include <bsp.h>

int main()
{
    int n = bsp_nprocs(); 
    int i = bsp_pid();

    // set char
    char* a = 0x1000;
    (*a) = 'h';

    return 0;
}
