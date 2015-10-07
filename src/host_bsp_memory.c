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

#define MALLOC_FUNCTION_PREFIX
#include "extmem_malloc_implementation.cpp"
#include "host_bsp_private.h"

#include <stdio.h>

//
// Host version of ebsp memory allocation functions
// Can only be used when epiphany cores are not running
//

// Should be called once on host after state.host_dynmem_addr has been set
void ebsp_malloc_init() {
    return _init_malloc_state(state.host_dynmem_addr, DYNMEM_SIZE);
}

void* ebsp_ext_malloc(unsigned int nbytes) {
    return _malloc(state.host_dynmem_addr, nbytes);
}

void ebsp_free(void* ptr) { return _free(state.host_dynmem_addr, ptr); }

int ebsp_write(int pid, void* src, off_t dst, int size) {
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    if (e_write(&state.dev, prow, pcol, dst, src, size) != size) {
        fprintf(stderr,
                "ERROR: e_write(dev,%d,%d,%p,%p,%d) failed in ebsp_write.\n",
                prow, pcol, (void*)dst, (void*)src, size);
        return 0;
    }
    return 1;
}

int ebsp_read(int pid, off_t src, void* dst, int size) {
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    if (e_read(&state.dev, prow, pcol, src, dst, size) != size) {
        fprintf(stderr,
                "ERROR: e_read(dev,%d,%d,%p,%p,%d) failed in ebsp_read.\n",
                prow, pcol, (void*)src, (void*)dst, size);
        return 0;
    }
    return 1;
}

int _write_core_syncstate(int pid, int syncstate) {
    return ebsp_write(pid, &syncstate, (off_t)state.combuf.syncstate_ptr, 4);
}

int _write_extmem(void* src, off_t offset, int size) {
    if (e_write(&state.emem, 0, 0, offset, src, size) != size) {
        fprintf(stderr, "ERROR: _write_extmem(src,%p,%d) failed.\n",
                (void*)offset, size);
        return 0;
    }
    return 1;
}

