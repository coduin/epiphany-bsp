#include <e_bsp.h>
#include "e-lib.h"

#define CORE_ID e_group_config.core_row*e_group_config.group_cols+e_group_config.core_col
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    // set char */
    char* a = (void*)0x7050;
    (*a) = hmsg[p];
//hmsg[p];
    int* po = (void*)0x7100;
    int tmp=CORE_ID;
    (*po) = tmp;

    return 0;
}
