/*
File: e_bsp.c

This file is part of the Epiphany BSP library.

Copyright (C) 2014 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/

#include "e_bsp_private.h"
#include <stdlib.h>

// Allocated chunks are 8-byte (64-bit) aligned
// so that they can be used by for example the DMA engine
#define CHUNK_SIZE 8

// Every chunk starts with the follwoing structs
// which must be exactly 8 bytes, after which the
// actual allocated memory starts
// This means every malloc will use up at least 16 bytes
typedef struct {
    uint32_t chunk_count;
    uint32_t padding;
} memory_object;

#define DYNMEM_START   (COMMBUF_EADDR + sizeof(ebsp_comm_buf))
#define DYNMEM_SIZE    (SHARED_MEM_END - DYNMEM_START)

// At DYNMEM_START there is first a long bitmask that indicates which chunks
// are currently in use. For n integers in the bitmask we can cover
// 32*CHUNK_SIZE*n bytes of memory. But these n integers use 4*n space of
// that as well. That means, if we have k bytes of memory, we need n so that
// 4*n + 32*CHUNK_SIZE*n = k
// Meaning n = k / (4+32*CHUNK_SIZE)
#define total_bitmask_ints (DYNMEM_SIZE / (4 + 32*CHUNK_SIZE))
// The allocated memory starts at
// chunk_roundup(DYNMEM_START + total_bitmask_ints*4)
// Round up to the next multiple of CHUNK_SIZE only if not a multiple yet
inline uint32_t chunk_roundup(uint32_t a)
{
    // Compiler optimizes this function to (((a+7)>>3)<<3)
    // I also tested ((a+7) & ~7)
    // but this is 4 extra bytes of assembly and
    // takes exactly 1 extra clockcycle
    return ((a+CHUNK_SIZE-1)/CHUNK_SIZE)*CHUNK_SIZE;
}

// Divide by chunksize but rounded up so that
// it is the amount of chunks this size takes up
inline uint32_t chunk_division(uint32_t a)
{
    // Optimized to ((a+7)>>3)
    return ((a + CHUNK_SIZE - 1)/CHUNK_SIZE);
}

// This value is actually known at compile time but becomes a long define
// to write explicitly in the source. Making it inline will optimize
// to a constant at compile time
inline void* get_alloc_base()
{
    return (void*)chunk_roundup(DYNMEM_START + total_bitmask_ints*4);
}

// ebsp_ext_malloc wraps this in a mutex
void* EXT_MEM_TEXT _malloc(uint32_t nbytes)
{
    nbytes += sizeof(memory_object);
    uint32_t chunk_count = chunk_division(nbytes);
    uint32_t *bitmasks = (uint32_t*)DYNMEM_START;

    // Search for a sequence of chunk_count zero bits
    uint32_t start_mask = 0;
    uint32_t start_bit = 0;
    uint32_t chunks_left = chunk_count;
    for (uint32_t i = 0; i < total_bitmask_ints; ++i)
    {
        uint32_t mask = bitmasks[i];
        if (mask == 0)
        {
            // All 32 bits (chunks) of this mask are available
            // so we can handle them all at once
            if (chunks_left <= 32)
            {
                chunks_left = 0;
                break;
            }
            else
            {
                chunks_left -= 32;
                continue;
            }
        }
        else
        {
            // Mask is not empty. We will need to parse all individual bits
            for (uint32_t j = 0; j < 32; ++j)
            {
                if (mask & 1)
                {
                    // memory not available
                    // restart right after this chunk
                    start_mask = i;
                    start_bit = j + 1;
                    // wrap around if needed
                    if (start_bit == 32){ ++start_mask; start_bit = 0; }
                    chunks_left = chunk_count; // reset
                }
                else
                {
                    chunks_left--;
                    if (chunks_left == 0)
                        break;
                }
                mask >>= 1;
            }
        }
    }
    // Unable to find free space
    if (chunks_left != 0) return 0;

    // Fill all the bits that we found starting at start_mask,start_bit
    chunks_left = chunk_count;
    uint32_t bit = (1U << start_bit);
    for (uint32_t i = start_mask; chunks_left != 0; i++)
    {
        uint32_t mask = bitmasks[i];
        for (; bit != 0 && chunks_left != 0; bit <<= 1)
        {
            mask |= bit;
            chunks_left--;
        }
        bitmasks[i] = mask;
        bit = 1;
    }
    
    // Bits have been filled. Now put a memory_object at the allocated space
    void* ptr = get_alloc_base() + CHUNK_SIZE*(start_mask*32+start_bit);
    ((memory_object*)ptr)->chunk_count = chunk_count;
    return ptr + sizeof(memory_object);
}

void EXT_MEM_TEXT _free(void* ptr)
{
    ptr -= sizeof(memory_object);
    uint32_t chunk_start = ((unsigned)(ptr - get_alloc_base()))/CHUNK_SIZE;
    uint32_t chunk_count = ((memory_object*)ptr)->chunk_count;

    uint32_t *bitmasks = (uint32_t*)DYNMEM_START;

    uint32_t chunk_mask = chunk_start/32;
    uint32_t bit = chunk_start%32;
    for (; chunk_count != 0; ++chunk_mask)
    {
        uint32_t mask = bitmasks[chunk_mask];
        for(; bit < 32 && chunk_count != 0; ++bit)
        {
            mask &= ~(1U << bit);
            chunk_count--;
        }
        bitmasks[chunk_mask] = mask;
        bit = 0;
    }
    return;
}

void EXT_MEM_TEXT _init_malloc_state()
{
    if (coredata.pid == 0)
    {
        uint32_t *bitmasks = (uint32_t*)DYNMEM_START;
        for (uint32_t i = 0; i < total_bitmask_ints; ++i)
            bitmasks[i] = 0;
    }
}

void* EXT_MEM_TEXT ebsp_ext_malloc(unsigned int nbytes)
{
    void *ret = 0;
    e_mutex_lock(0, 0, &coredata.malloc_mutex);
    ret = _malloc(nbytes);
    e_mutex_unlock(0, 0, &coredata.malloc_mutex);
    return ret;
}

void EXT_MEM_TEXT ebsp_free(void* ptr)
{
    e_mutex_lock(0, 0, &coredata.malloc_mutex);
    _free(ptr);
    e_mutex_unlock(0, 0, &coredata.malloc_mutex);
}
