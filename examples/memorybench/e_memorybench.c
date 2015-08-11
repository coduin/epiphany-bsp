#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();
    int p = bsp_pid();
    int n = bsp_nprocs();

    //
    // Core <-> External memory
    //
    const long chunk_size = 2048;
    const long dword_count = chunk_size / sizeof(long long);

    long long* extmem_buffer = ebsp_ext_malloc(chunk_size);
    long long local_buffer[dword_count];

    // Write speed, one core at a time
    
    if (p == 0) {
        ebsp_message("write extmem nodma nonbusy");
    }
    for (int iter=0; iter < 32; iter++) {
        bsp_sync();
        if (iter % n == p)
        {
            for (int h = 0; h <= dword_count; h++) {
                ebsp_raw_time(); //Reset timer
                for (int i = 0; i < h; i++)
                    extmem_buffer[i] = local_buffer[i];
                int timer = ebsp_raw_time();
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Read speed, one core at a time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read extmem nodma nonbusy");
    }
    for (int iter=0; iter < 32; iter++) {
        bsp_sync();
        if (iter % n == p)
        {
            for (int h = 0; h <= dword_count; h++) {
                //Reset timer
                ebsp_raw_time();
                for (int i = 0; i < h; i++)
                    local_buffer[i] = extmem_buffer[i];
                int timer = ebsp_raw_time();
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Write speed, all cores at the same time

    bsp_sync();
    int timings[dword_count+1];
    if (p == 0) {
        ebsp_message("write extmem nodma busy");
    }
    for (int iter=0; iter < 10; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            ebsp_raw_time();
            for (int i = 0; i < h; i++)
                extmem_buffer[i] = local_buffer[i];
            timings[h] = ebsp_raw_time();
            // 'sleep'
            //for (int i = 0; i < dword_count; i++)
            //    local_buffer[i] = (local_buffer[dword_count - i - 1] * 5) % (p+100);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }
    
    // Read speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read extmem nodma busy");
    }
    for (int iter=0; iter < 10; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            ebsp_raw_time();
            for (int i = 0; i < h; i++)
                local_buffer[i] = extmem_buffer[i];
            timings[h] = ebsp_raw_time();
            // 'sleep'
            //for (int i = 0; i < dword_count; i++)
            //    local_buffer[i] = (local_buffer[dword_count - i - 1] * 5) % (p+100);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }

    bsp_end();
    
    return 0;
}

