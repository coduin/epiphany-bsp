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

//
// Taken from e_dma_copy source
//

extern unsigned dma_data_size[8];

#define local_mask (0xfff00000)

extern e_dma_desc_t _dma_copy_descriptor_;

int dma_copy(void *dst, void *src, size_t n)
{
    e_dma_id_t chan;
    unsigned   index;
    unsigned   shift;
    unsigned   stride;
    unsigned   config;
    int        ret_val;

    if (n == 0) return 0;

    chan  = E_DMA_0;

    index = (((unsigned) dst) | ((unsigned) src) | ((unsigned) n)) & 7;

    config = E_DMA_MASTER | E_DMA_ENABLE | dma_data_size[index];
    if ((((unsigned) dst) & local_mask) == 0)
        config = config | E_DMA_MSGMODE;
    shift = dma_data_size[index] >> 5;
    stride = 0x10001 << shift;

    // TODO: add e_dma_wait()!!!
    _dma_copy_descriptor_.config       = config;
    _dma_copy_descriptor_.inner_stride = stride;
    _dma_copy_descriptor_.count        = 0x10000 | (n >> shift);
    _dma_copy_descriptor_.outer_stride = 0; // stride;
    _dma_copy_descriptor_.src_addr     = src;
    _dma_copy_descriptor_.dst_addr     = dst;

    ret_val = e_dma_start(&_dma_copy_descriptor_, chan);

    volatile unsigned* reg_addr = e_get_global_address(e_group_config.core_row, e_group_config.core_col, (void*)E_REG_DMA0STATUS);

    while ( (*reg_addr) & 0xf );

    return ret_val;
}


// count is measured in longlongs
unsigned int dma_transfer(void* destination, const void* source, unsigned int count)
{
    ebsp_raw_time(); //reset timer
    dma_copy(destination, (void*)source, count*sizeof(long long));
    return ebsp_raw_time();
}

#define chunk_size  2048
#define dword_count (chunk_size / sizeof(long long))

long long* extmem_buffer;
long long local_buffer[dword_count];

const int samples = 1;
int timings[dword_count+1];

int p, n;

//Integrity check
void check_buffers()
{
    for (unsigned int i = 0; i < dword_count;  i++)
    {
        if (local_buffer[i] != i)
        {
            bsp_abort("Local buffer is corrupted!!!");
            break;
        }
        if (extmem_buffer[i] != i)
        {
            bsp_abort("External buffer is corrupted!!!");
            break;
        }
    }
}

void* get_remote_addr(int pid, const void *addr)
{
    return e_get_global_address(pid / e_group_config.group_cols, pid % e_group_config.group_cols, addr);
}

void measure(const char* target_string, void* target_buffer)
{
    // Write speed, one core at a time
    
    bsp_sync();
    if (p == 0) {
        ebsp_message("write %s nodma nonbusy", target_string);
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = reverse_transfer(target_buffer, local_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    check_buffers();

    // Read speed, one core at a time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read %s nodma nonbusy", target_string);
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = transfer(local_buffer, target_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    check_buffers();

    // Write speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("write %s nodma busy", target_string);
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = transfer(target_buffer, local_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }

    check_buffers();

    // Read speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read %s nodma busy", target_string);
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = transfer(local_buffer, target_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }

    check_buffers();

    //========================================================================
    // DMA
    //========================================================================

    // Write speed, one core at a time
    
    bsp_sync();
    if (p == 0) {
        ebsp_message("write %s dma nonbusy", target_string);
    }
    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = dma_transfer(target_buffer, local_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }

    check_buffers();

    // Read speed, one core at a time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read %s dma nonbusy", target_string);
    }

    for (int iter=0; iter < samples; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            bsp_sync();
            if (i != p) continue;
            for (int h = 0; h <= dword_count; h++) {
                int timer = dma_transfer(local_buffer, target_buffer, h);
                ebsp_message("%d %d", h, timer);
            }
        }
    }
    check_buffers();

    //// Write speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("write %s dma busy", target_string);
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = dma_transfer(target_buffer, local_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }
    check_buffers();
    
    // Read speed, all cores at the same time

    bsp_sync();
    if (p == 0) {
        ebsp_message("read %s dma busy", target_string);
    }
    for (int iter=0; iter < samples; iter++) {
        for (int h = 0; h <= dword_count; h++) {
            bsp_sync();
            timings[h] = dma_transfer(local_buffer, target_buffer, h);
            bsp_sync();
        }
        for (int h = 0; h <= dword_count; h++ )
            ebsp_message("%d %d", h, timings[h]);
    }
    check_buffers();
}

int main()
{
    bsp_begin();
    p = bsp_pid();
    n = bsp_nprocs();

    extmem_buffer = ebsp_ext_malloc(chunk_size);

    // Integrity checks
    for (int i = 0; i < dword_count; i++)
        local_buffer[i] = i;

    // Special case: extmem burst mode
    bsp_sync();
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
    check_buffers();

    measure("extmem", extmem_buffer);

    void* neighbor_buffer = get_remote_addr((p+1)%n, &local_buffer);
    measure("core", neighbor_buffer);

    bsp_end();
    
    return 0;
}

