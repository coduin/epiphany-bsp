// This program is based on bspbench from BSPedupack by Rob Bisseling, copyrighted in 2004
// BSPedupack is under the GNU General Public License

#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>
 
/*  This program measures p, r, g, and l of a BSP computer
    using bsp_put for communication.
*/

#define MEGA 1000000.0

int P; /* number of processors requested */

int main(int argc, char **argv){
    if(!bsp_init("e_bspbench.srec", argc, argv)) {
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
    ebsp_set_tagsize(&tagsize);

    printf("Using %i processors!\n", bsp_nprocs()); fflush(stdout);
    ebsp_spmd();
    printf("ebsp_spmd finished.\n");

    //Get r, g and l
    float r,g,l;

    int packets, accum_bytes, status, tag;
    ebsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++)
    {
        ebsp_get_tag(&status, &tag);
        if (tag == 'r')
            ebsp_move(&r, sizeof(int));
        else if (tag == 'g')
            ebsp_move(&g, sizeof(int));
        else if (tag == 'l')
            ebsp_move(&l, sizeof(int));
        else if (tag == 'h')
        {
            float *results = (float*)malloc(status);
            ebsp_move(results, status);
            printf("h-relation timings:\n");
            int count = status/sizeof(float);
            for (int j = 0; j < count; j++)
                printf("%d %f\n", j, results[j]);
            free(results);
        }
        else
        {
            // pop from message queue
            ebsp_move(0, 0);
        }
    }

    printf("The bottom line for this BSP computer is:\n");
    printf("p= %d, r= %.3lf Mflop/s, g= %E, l= %E\n",
         bsp_nprocs(),r/MEGA,g,l);
    bsp_end();
    
    return 0;
} /* end main */
