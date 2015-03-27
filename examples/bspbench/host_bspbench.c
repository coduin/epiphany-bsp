// This program is based on bspbench from BSPedupack by Rob Bisseling, copyrighted in 2004
// BSPedupack is under the GNU General Public License

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
    //printf("How many processors do you want to use?\n"); fflush(stdout);
    //scanf("%d",&P);
    //if (P > bsp_nprocs()){
    //    printf("Sorry, not enough processors available.\n");
    //    return -1;
    //}
    if(!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[BSPBENCH] bsp_begin() failed\n");
    }

    int tagsize = sizeof(int);
    int tag = 5;
    char payload[16];
    int nbytes = 16;

    ebsp_set_tagsize(&tagsize);
    for (int i = 0; i < bsp_nprocs(); i++)
    {
        payload[0] = i;
        ebsp_senddown(i, &tag, &payload, nbytes);
    }

    printf("Using %i processors!\n", bsp_nprocs()); fflush(stdout);
    ebsp_spmd();
    printf("ebsp_spmd finished.\n");

    //Get p, r, g and l
    int p;
    float r,g,l,l0,g0;
    co_read(0, (off_t)0x6000, &p, sizeof(int));
    co_read(0, (off_t)0x6010, &r, sizeof(float));
    co_read(0, (off_t)0x6020, &g, sizeof(float));
    co_read(0, (off_t)0x6030, &l, sizeof(float));
    co_read(0, (off_t)0x6040, &g0, sizeof(float));
    co_read(0, (off_t)0x6050, &l0, sizeof(float));

    int i;
    int j; 
    for(j=0; j<=64; j++){
        float tmp;
        co_read(0, (off_t)(0x4000+j*sizeof(float)), &tmp, sizeof(float));
        //printf("h[%d]=%E\n",j,tmp);
        printf("%d %f\n",j,tmp);
    }

    printf("The bottom line for this BSP computer is:\n");
    printf("p= %d, r= %.3lf Mflop/s, g= %E, l= %E, g0= %E, l0= %E\n",
         p,r/MEGA,g,l, g0, l0);
    bsp_end();
    
    return 0;
} /* end main */
