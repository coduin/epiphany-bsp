#include <bsp.h>

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    int base = 0x7000;
    int chunk = (*(int*)(base));
    int i = 0;
    int sum = 0;
    for(i = 0; i < chunk; ++i) {
        sum += (*(int*)(base + 4 + 4*i)) * (*(int*)(base + 4 + 4*(chunk + i)));
    }

    int* loc = (int*)(0x7004 + 32*8);
    (*loc) = sum;

    return 0;
}
