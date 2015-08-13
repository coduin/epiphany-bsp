/*
File: host_bsp_memory.c

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


#define MALLOC_FUNCTION_PREFIX 
#include "extmem_malloc_implementation.cpp"
#include "common.h"

//
//Host version of ebsp memory allocation functions
//Can only be used when epiphany cores are not running
//

void* malloc_base;

//Should be called once on host
void ebsp_malloc_init(void* external_memory_base)
{
    malloc_base = external_memory_base;
    return _init_malloc_state(malloc_base, DYNMEM_SIZE);
}

void* ebsp_ext_malloc(unsigned int nbytes)
{
    return _malloc(malloc_base, nbytes);
}

void ebsp_free(void* ptr)
{
    return _free(malloc_base, ptr);
}
