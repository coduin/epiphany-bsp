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

// To avoid burst mode of the e-link
unsigned int reverse_transfer(void* destination, const void* source, unsigned int count)
{
    ebsp_raw_time(); //reset timer
    long long* dst = (long long*)destination;
    const long long* src = (const long long*)source;
    dst += count;
    src += count;
    while(count--)
        *--dst = *--src;
    return ebsp_raw_time();
}

// count is measured in longlongs
unsigned int dma_transfer(void* destination, const void* source, unsigned int count)
{
    ebsp_raw_time(); //reset timer
    e_dma_copy(destination, (void*)source, count*sizeof(long long));
    return ebsp_raw_time();
}

//Integrity check
void check_buffer(const long long* buffer, unsigned int count)
{
    for (unsigned int i = 0; i < count;  i++)
    {
        if (buffer[i] != i)
        {
            bsp_abort("Buffer %p is corrupted!!!", buffer);
            break;
        }
    }
}

int main()
{
    bsp_begin();
    int p = bsp_pid();
    int n = bsp_nprocs();

    //
    // Core <-> External memory
    //
    const long chunk_size = 4096;
    const long dword_count = chunk_size / sizeof(long long);

    long long* extmem_buffer = ebsp_ext_malloc(chunk_size);
    long long local_buffer[dword_count];

    // Integrity checks
    for (int i = 0; i < dword_count; i++)
        local_buffer[i] = i;

    const int samples = 1;
    int timings[dword_count+1];

    // Write speed, one core at a time
    
    if (p == 0) {
        ebsp_message("write extmem nodma burst");
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
                int timer = reverse_transfer(extmem_buffer, local_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }


    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);

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

    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);

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

    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);

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

    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);

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

    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);

    // Read speed, one core at a time

//TODO: Figure out why DMA never returns
//    bsp_sync();
//    if (p == 0) {
//        ebsp_message("read extmem dma nonbusy");
//    }
//
//    for (int iter=0; iter < samples; iter++)
//    {
//        for (int i = 0; i < n; i++)
//        {
//            bsp_sync();
//            if (i != p) continue;
//            for (int h = 0; h <= dword_count; h++) {
//                int timer = transfer(local_buffer, extmem_buffer, h);
//                ebsp_message("%d %d", h, timer);
//            }
//        }
//    }
//
//    check_buffer(extmem_buffer, dword_count);
//    check_buffer(local_buffer, dword_count);

    //// Write speed, all cores at the same time

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

    check_buffer(extmem_buffer, dword_count);
    check_buffer(local_buffer, dword_count);
    
    // Read speed, all cores at the same time

//    bsp_sync();
//    if (p == 0) {
//        ebsp_message("read extmem dma busy");
//    }
//    for (int iter=0; iter < samples; iter++) {
//        for (int h = 0; h <= dword_count; h++) {
//            bsp_sync();
//            timings[h] = transfer(local_buffer, extmem_buffer, h);
//            bsp_sync();
//        }
//        for (int h = 0; h <= dword_count; h++ )
//            ebsp_message("%d %d", h, timings[h]);
//    }
//
    bsp_end();
    
    return 0;
}

