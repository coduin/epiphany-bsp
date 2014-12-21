#include <bsp.h>

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    int base = 0x6000;
    int chunk = (*(int*)(base));
    int i = 0;
    int sum = 0;
    for(i = 0; i < chunk; ++i) {
        sum += (*(int*)(base + 4 + 4*i)) * (*(int*)(base + 4 + 4*(chunk + i)));
    }

    int* loc = (int*)(base + 4 + 32*8);
    (*loc) = sum;

    return 0;
}
