#include <e_bsp.h>
#include "e-lib.h"

// Fast transfer of 8-byte aligned data only
// count is measured in longlongs
// Returns transfer time
unsigned int transfer(void* destination, const void* source, unsigned int count)
{
    ebsp_raw_time(); //reset timer
    long long* dst = (long long*)destination;
    const long long* src = (const long long*)source;
    while(count--)
        *dst++ = *src++;
    return ebsp_raw_time();
}

// count is measured in longlongs
unsigned int dma_transfer(void* destination, const void* source, unsigned int count)
{
    ebsp_raw_time(); //reset timer
    e_dma_copy(destination, (void*)source, count*sizeof(long long));
    return ebsp_raw_time();
}

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

    const int samples = 1;
    int timings[dword_count+1];

    // Write speed, one core at a time
    
    if (p == 0) {
        ebsp_message("write extmem nodma nonbusy");
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = transfer(extmem_buffer, local_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Read speed, one core at a time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read extmem nodma nonbusy");
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = transfer(local_buffer, extmem_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Write speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("write extmem nodma busy");
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = transfer(extmem_buffer, local_buffer, h);
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
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = transfer(local_buffer, extmem_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }

    //========================================================================
    // DMA
    //========================================================================

    // Write speed, one core at a time
    
    bsp_sync();
    if (p == 0) {
        ebsp_message("write extmem dma nonbusy");
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = dma_transfer(extmem_buffer, local_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Read speed, one core at a time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read extmem dma nonbusy");
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = dma_transfer(local_buffer, extmem_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    // Write speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("write extmem dma busy");
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = dma_transfer(extmem_buffer, local_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }
    
    // Read speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read extmem dma busy");
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = dma_transfer(local_buffer, extmem_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }

    bsp_end();
    
    return 0;
}

