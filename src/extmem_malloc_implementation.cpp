/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
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

// This file assumes the following preprocessor variable is defined:
// MALLOC_FUNCTION_PREFIX - _malloc and _free are prefixed with this
//                        - Empty variable on the host
//                        - On the Epiphany, puts function itself in external
//                          memory

#include <stdint.h>

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

//
// Layout of memory:
//     base + 0x00: uint32_t total_bitmask_ints
//     base + 0x04: uint32_t bitmasks[total_bitmask_ints]
//     base + 0x??: allocated memory
//
// Use get_alloc_base to get the ?? address
//

// At base+4 there is first a long bitmask that indicates which chunks
// are currently in use. For n integers in the bitmask we can cover
// 32*CHUNK_SIZE*n bytes of memory. But these n integers use 4*n space of
// that as well. That means, if we have k bytes of memory, we need n so that
// 4*n + 32*CHUNK_SIZE*n = k
// Meaning n = k / (4+32*CHUNK_SIZE)
#define compute_total_bitmask_ints(size) ((size - 4) / (4 + 32 * CHUNK_SIZE))

// The allocated memory starts at
// chunk_roundup(base + 4 + total_bitmask_ints*4)
// Round up to the next multiple of CHUNK_SIZE only if not a multiple yet
inline uint32_t chunk_roundup(uint32_t a) {
    // Compiler optimizes this function to (((a+7)>>3)<<3)
    // I also tested ((a+7) & ~7)
    // but this is 4 extra bytes of assembly and
    // takes exactly 1 extra clockcycle
    return ((a + CHUNK_SIZE - 1) / CHUNK_SIZE) * CHUNK_SIZE;
}

// rounded-up divide
// Divide by chunksize but rounded up so that
// it is the amount of chunks this size takes up
inline uint32_t chunk_division(uint32_t a) {
    // Optimized to ((a+7)>>3)
    return ((a + CHUNK_SIZE - 1) / CHUNK_SIZE);
}

inline uint32_t get_bitmask_count(const void* base) { return *(uint32_t*)base; }

inline uint32_t* get_bitmasks(const void* base) {
    return (uint32_t*)(base + 4);
}

inline void* get_alloc_base(const void* base) {
    return (void*)chunk_roundup((uint32_t)(base + 4 * (1 + *(uint32_t*)base)));
}

// ebsp_ext_malloc wraps this in a mutex
void* MALLOC_FUNCTION_PREFIX _malloc(void* base, uint32_t nbytes) {
    nbytes += sizeof(memory_object);
    uint32_t chunk_count = chunk_division(nbytes);

    uint32_t total_bitmask_ints = get_bitmask_count(base);
    uint32_t* bitmasks = get_bitmasks(base);

    // Search for a sequence of chunk_count zero bits
    uint32_t start_mask = 0;
    uint32_t start_bit = 0;
    uint32_t chunks_left = chunk_count;
    for (uint32_t i = 0; i < total_bitmask_ints; ++i) {
        uint32_t mask = bitmasks[i];
        if (mask == 0) {
            // All 32 bits (chunks) of this mask are available
            // so we can handle them all at once
            if (chunks_left <= 32) {
                chunks_left = 0;
                break;
            } else {
                chunks_left -= 32;
                continue;
            }
        } else if (mask == -1) {
            // All 32 bits (chunks) are in use
            // So start at least AFTER this one
            start_mask = i + 1;
            start_bit = 0;
        } else {
            // Mask is not empty. We will need to parse all individual bits
            for (uint32_t j = 0; j < 32; ++j) {
                if (mask & 1) {
                    // memory not available
                    // restart right after this chunk
                    start_mask = i;
                    start_bit = j + 1;
                    // wrap around if needed
                    if (start_bit == 32) {
                        ++start_mask;
                        start_bit = 0;
                    }
                    chunks_left = chunk_count; // reset
                } else {
                    chunks_left--;
                    if (chunks_left == 0)
                        break;
                }
                mask >>= 1;
            }
        }
    }
    // Unable to find free space
    if (chunks_left != 0)
        return 0;

    // Fill all the bits that we found starting at start_mask,start_bit
    chunks_left = chunk_count;
    uint32_t bit = (1U << start_bit);
    for (uint32_t i = start_mask; chunks_left != 0; i++) {
        uint32_t mask = bitmasks[i];
        for (; bit != 0 && chunks_left != 0; bit <<= 1) {
            mask |= bit;
            chunks_left--;
        }
        bitmasks[i] = mask;
        bit = 1;
    }

    // Bits have been filled. Now put a memory_object at the allocated space
    void* ptr =
        get_alloc_base(base) + CHUNK_SIZE * (start_mask * 32 + start_bit);
    ((memory_object*)ptr)->chunk_count = chunk_count;
    return ptr + sizeof(memory_object);
}

void MALLOC_FUNCTION_PREFIX _free(void* base, void* ptr) {
    ptr -= sizeof(memory_object);
    uint32_t chunk_start =
        ((unsigned)(ptr - get_alloc_base(base))) / CHUNK_SIZE;
    uint32_t chunk_count = ((memory_object*)ptr)->chunk_count;

    uint32_t* bitmasks = get_bitmasks(base);

    uint32_t chunk_mask = chunk_start / 32;
    uint32_t bit = chunk_start % 32;
    for (; chunk_count != 0; ++chunk_mask) {
        uint32_t mask = bitmasks[chunk_mask];
        for (; bit < 32 && chunk_count != 0; ++bit) {
            mask &= ~(1U << bit);
            chunk_count--;
        }
        bitmasks[chunk_mask] = mask;
        bit = 0;
    }
    return;
}

// Initializes the malloc table
void MALLOC_FUNCTION_PREFIX _init_malloc_state(void* base, uint32_t size) {
    uint32_t total_bitmask_ints = compute_total_bitmask_ints(size);

    // First we store the AMOUNT of bitmask ints
    // Then there is the bitmask ints themselves
    // Then there is the allocated memory
    uint32_t* ptr = (uint32_t*)base;
    *ptr++ = total_bitmask_ints;
    while (total_bitmask_ints--)
        *ptr++ = 0;
}

// For debug purposes
void MALLOC_FUNCTION_PREFIX
_get_malloc_info(void* base, uint32_t* used, uint32_t* free) {
    uint32_t total_bitmask_ints = get_bitmask_count(base);
    uint32_t* bitmasks = get_bitmasks(base);

    uint32_t bits_in_use = 0;
    uint32_t bits_free = 0;
    for (uint32_t i = 0; i < total_bitmask_ints; ++i) {
        uint32_t mask = bitmasks[i];
        if (mask == 0) {
            bits_free += 32;
            continue;
        } else if (mask == -1) {
            bits_in_use += 32;
            continue;
        } else {
            // Mask is not empty. We will need to parse all individual bits
            for (uint32_t j = 0; j < 32; ++j) {
                if (mask & 1) {
                    bits_in_use++;
                } else {
                    bits_free++;
                }
                mask >>= 1;
            }
        }
    }
    *used = bits_in_use * CHUNK_SIZE;
    *free = bits_free * CHUNK_SIZE;
}
