#include <bsp_host.h>
#include <stdlib.h>
#include <stdio.h> 

int main(int argc, char **argv)
{
    bsp_init("bin/dotproduct.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // for loops
    int pid = 0;
    int i;

    // allocate two random vectors of length 512
    int l = 512;
    int* a = (int*)malloc(sizeof(int) * l);
    int* b = (int*)malloc(sizeof(int) * l);
    for(i = 0; i < l; ++i) {
        a[i] = i;
        b[i] = 2*i;
    }

    // partition and write to processors
    int chunk = l / bsp_nprocs();
    printf("chunk: %i\n", chunk);

    // write in memory bank 4
    int base = 0x6000;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        int prow, pcol;
        _get_p_coords(pid, &prow, &pcol);
        e_write(&(_get_state()->dev), prow, pcol, (off_t)base, &chunk, sizeof(int));
        for(i = 0; i < chunk; ++i) {
            e_write(&(_get_state()->dev), prow, pcol, (off_t)(base + 4 + 4*i), &a[i + chunk*pid], 4);
            e_write(&(_get_state()->dev), prow, pcol, (off_t)(base + 4 + 4*chunk + 4*i), &b[i + chunk*pid], 4);
        }
    }
    
    // run dotproduct
    spmd_epiphany();

    // read output
    int* result = malloc(sizeof(int) * bsp_nprocs());
    int sum = 0;
    printf("proc \t mem_loc \t partial_sum\n");
    printf("---- \t ------- \t -----------\n");
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        int pcol, prow;
        _get_p_coords(pid, &prow, &pcol);
        e_read(&(_get_state()->dev), prow, pcol, (off_t)(base + 4 + 8*chunk), &result[pid], sizeof(int));
        printf("%i: \t 0x%x \t %i\n", pid, base + 4 + 8*chunk, result[pid]);
        sum += result[pid];
    }

    printf("SUM: %i\n", sum);

    free(a);
    free(b);
    free(result);

    // finalize
    bsp_end();

    return 0;
}
