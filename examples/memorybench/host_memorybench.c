#include <host_bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define BENCH_HOST

#ifdef BENCH_HOST

#define __USE_XOPEN2K
#define __USE_POSIX199309 1
#include <time.h>

struct timespec ts_start, ts_end;

void reset()
{
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
}

long long getElapsedNanoSec()
{
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return ((ts_end.tv_sec - ts_start.tv_sec) * 1e9  +
            (ts_end.tv_nsec - ts_start.tv_nsec));
}

void* ebsp_ext_malloc(unsigned int nbytes);
void ebsp_free(void* ptr);

#endif

int main(int argc, char **argv)
{
    if(!bsp_init("e_memorybench.srec", argc, argv)) {
        fprintf(stderr, "[BSPBENCH] bsp_init() failed\n");
        return -1;
    }
    if (!bsp_begin(bsp_nprocs())) {
        fprintf(stderr, "[BSPBENCH] bsp_begin() failed\n");
    }

#ifdef BENCH_HOST
    long long time_loop, time_memcpy, time_flops;

    const unsigned chunk_size = 4*1024*1024;
    void* ext_ptr = ebsp_ext_malloc(chunk_size);
    void* local_ptr = malloc(chunk_size);

    if (!ext_ptr)
    {
        fprintf(stderr, "extmem_malloc error");
    }
    else if (!local_ptr)
    {
        fprintf(stderr, "local malloc error");
    }
    else
    {
        // Timing: copy to external memory in a long long loop
        {
            const unsigned dword_count = chunk_size / sizeof(long long);
            long long* dst = (long long*)ext_ptr;
            long long* src = (long long*)local_ptr;
            reset();
            for (unsigned i = 0; i < dword_count; i++)
                *dst++ = *src++;
            time_loop = getElapsedNanoSec();
        }

        // Timing: memcpy to external memory
        {
            reset();
            memcpy(ext_ptr, local_ptr, chunk_size);
            time_memcpy = getElapsedNanoSec();
        }

        // Timing: floating point operations
        {
            float* floats = (float*)local_ptr;
            unsigned float_count = chunk_size / sizeof(float);

            reset();
            floats[0] = 1.0f;
            floats[1] = 1.0f;
            floats[2] = 1.0f;
            for (unsigned i = 3; i < float_count; i++)
            {
                floats[i] = floats[i-1] + floats[i-2] * floats[i-3];
            }
            time_flops = getElapsedNanoSec();
        }

        ebsp_free(ext_ptr);
        free(local_ptr);
    }

    printf("Host timing info:\n");
    printf("copy by loop:\t%lld ns\t%fms\n",time_loop,1e-6f*time_loop);
    printf("copy by memcpy:\t%lld ns\t%fms\n",time_memcpy,1e-6f*time_memcpy);
    printf("flops loop:\t%lld ns\t%fms\n",time_flops,1e-6f*time_flops);
#else
    ebsp_spmd();
#endif
    bsp_end();
    
    return 0;
}
