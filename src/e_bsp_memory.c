/*
File: e_bsp_memory.c

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
#include "common.h"
#define MALLOC_FUNCTION_PREFIX EXT_MEM_TEXT

#include "extmem_malloc_implementation.cpp"

// This variable indicates end of global vars
// So 'end' until 'stack' can be used by malloc
extern int end;

// Called in bsp_begin by every core
void EXT_MEM_TEXT _init_local_malloc() {
    coredata.local_malloc_base = (void*)chunk_roundup((uint32_t)(&end + 8));
    uint32_t size = 0x8000 - (uint32_t)coredata.local_malloc_base;
    _init_malloc_state(coredata.local_malloc_base, size);
}

void* EXT_MEM_TEXT ebsp_ext_malloc(unsigned int nbytes) {
    void* ret = 0;
    e_mutex_lock(0, 0, &coredata.malloc_mutex);
    ret = _malloc((void*)E_DYNMEM_ADDR, nbytes);
    e_mutex_unlock(0, 0, &coredata.malloc_mutex);
    return ret;
}

void* EXT_MEM_TEXT ebsp_malloc(unsigned int nbytes) {
    void* ret = 0;
    ret = _malloc(coredata.local_malloc_base, nbytes);
    // Check if it does not overwrite the current stack position
    // Plus 128 bytes of margin
    if ((uint32_t)ret + nbytes + 128 > (uint32_t)&ret) // <-- only epiphany
    {
        _free(coredata.local_malloc_base, ret);
        ebsp_message("ERROR: allocation of %d bytes of local memory overwrites "
                     "the stack",
                     nbytes);
        return 0;
    }
    return ret;
}

void EXT_MEM_TEXT ebsp_free(void* ptr) {
    if (((unsigned)ptr) & 0xfff00000) {
        e_mutex_lock(0, 0, &coredata.malloc_mutex);
        _free((void*)E_DYNMEM_ADDR, ptr);
        e_mutex_unlock(0, 0, &coredata.malloc_mutex);
    } else {
        _free(coredata.local_malloc_base, ptr);
    }
}

void ebsp_memcpy(void* dest, const void* source, size_t nbytes) {
    unsigned bits = (unsigned)dest | (unsigned)source;
    if ((bits & 0x7) == 0) {
        // 8-byte aligned
        long long* dst = (long long*)dest;
        const long long* src = (const long long*)source;
        int count = nbytes >> 3;
        nbytes &= 0x7;
        while (count--)
            *dst++ = *src++;
        dest = (void*)dst;
        source = (void*)source;
    } else if ((bits & 0x3) == 0) {
        // 4-byte aligned
        uint32_t* dst = (uint32_t*)dest;
        const uint32_t* src = (const uint32_t*)source;
        int count = nbytes >> 2;
        nbytes &= 0x3;
        while (count--)
            *dst++ = *src++;
        dest = (void*)dst;
        source = (void*)source;
    }

    // do remaining bytes 1-byte aligned
    char* dst_b = (char*)dest;
    const char* src_b = (const char*)source;
    while (nbytes--)
        *dst_b++ = *src_b++;
}
