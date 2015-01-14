#include <host_bsp.h>
#include <stdlib.h>
#include <stdio.h> 

int main(int argc, char **argv)
{
    bsp_init("bin/e_dot_product.srec", argc, argv);
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
        co_write(pid, &chunk, (off_t)base, sizeof(int));
        for(i = 0; i < chunk; ++i) {
            co_write(pid, &a[i + chunk*pid], (off_t)(base + 4 + 4*i), sizeof(int));
            co_write(pid, &b[i + chunk*pid], (off_t)(base + 4 + 4*chunk + 4*i), sizeof(int));
        }
    }
    
    // run dotproduct
    ebsp_spmd();

    // read output
    int* result = malloc(sizeof(int) * bsp_nprocs());
    int sum = 0;
    printf("proc \t mem_loc \t partial_sum\n");
    printf("---- \t ------- \t -----------\n");
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        co_read(pid, (off_t)(base + 4 + 8*chunk), &result[pid], sizeof(int));
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
