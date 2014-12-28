#include <e-lib.h>

int main()
{
    int* wrt = (void*)0x7000;
    (*wrt) = 12346;

    return 0;
}
