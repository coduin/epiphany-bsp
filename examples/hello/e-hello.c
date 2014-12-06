#include <bsp.h>

int main()
{
    int n = bsp_nprocs(); 
    int i = bsp_pid();


    const char* hmsg = "Hello world!";

    // set char
    char* a = 0x1000;
    (*a) = hmsg[i];

    return 0;
}
