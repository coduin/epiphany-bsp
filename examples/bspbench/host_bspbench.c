#include <host_bsp.h>
#include <stdio.h>
 
/*  This program measures p, r, g, and l of a BSP computer
    using bsp_put for communication.
*/

#define MEGA 1000000.0

int P; /* number of processors requested */

int main(int argc, char **argv){
    if(!bsp_init("bin/e_bspbench.srec", argc, argv)) {
        fprintf(stderr, "[BSPBENCH] bsp_init() failed\n");
        return -1;
    }
    printf("How many processors do you want to use?\n"); fflush(stdout);
    scanf("%d",&P);
    if (P > bsp_nprocs()){
        printf("Sorry, not enough processors available.\n");
        return -1;
    } //
    if(!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[BSPBENCH] bsp_begin() failed\n");
    }
    printf("Using %i processors!", bsp_nprocs()); fflush(stdout);
    spmd_epiphany(); 

    //Get p, r, g and l
    int p,r,g,l;
    co_read(0, (off_t)0x6000, &p, 1);
    co_read(0, (off_t)0x6010, &r, 1);
    co_read(0, (off_t)0x6020, &g, 1);
    co_read(0, (off_t)0x6030, &l, 1);
    printf("The bottom line for this BSP computer is:\n");
    printf("p= %d, r= %.3lf Mflop/s, g= %.1lf, l= %.1lf\n",
         p,r/MEGA,g,l);
    bsp_end();
    
    return 0;
} /* end main */
