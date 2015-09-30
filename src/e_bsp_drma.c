/*
File: e_bsp_drma.c

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
#include <string.h>


const char err_pushreg_multiple[] EXT_MEM_RO =
    "BSP ERROR: multiple bsp_push_reg calls within one sync";

const char err_pushreg_overflow[] EXT_MEM_RO =
    "BSP ERROR: Trying to push more than MAX_BSP_VARS vars";

const char err_var_not_found[]    EXT_MEM_RO =
    "BSP ERROR: could not find bsp var %p";

const char err_pop_reg_not_implemented[]  EXT_MEM_RO =
    "BSP ERROR: Function bsp_pop_reg not implemented";

const char err_get_overflow[]     EXT_MEM_RO =
    "BSP ERROR: too many bsp_get requests per sync";

const char err_put_overflow[]     EXT_MEM_RO =
    "BSP ERROR: too many bsp_put requests per sync";

const char err_put_overflow2[]    EXT_MEM_RO =
    "BSP ERROR: too large bsp_put payload per sync";


// This incoroporates the bsp_var_list as well as
// the epiphany global address system
// The resulting address can be written to directly
void* _get_remote_addr(int pid, const void *addr, int offset)
{
    // Find the slot for our local pid
    // And return the entry for the remote pid including the epiphany mapping
    for (int slot = 0; slot < MAX_BSP_VARS; ++slot)
    {
        if (combuf->bsp_var_list[slot][coredata.pid] == addr)
        {
            // Address as registered by other core and as seen by other core
            unsigned uptr = (unsigned)combuf->bsp_var_list[slot][pid] + offset;

            // If it was global, then it is directly valid from here
            // If it was local, add the remote coreid in the highest 12 bits
            if ((uptr & 0xfff00000) == 0) //local
                uptr |= ((uint32_t)coredata.coreids[pid]) << 20;

            return (void*)uptr;
        }
    }
    ebsp_message(err_var_not_found, addr);
    return 0;
}

void EXT_MEM_TEXT bsp_push_reg(const void* variable, const int nbytes)
{
    if (coredata.var_pushed)
        return ebsp_message(err_pushreg_multiple);

    if (combuf->bsp_var_counter == MAX_BSP_VARS)
        return ebsp_message(err_pushreg_overflow);

    combuf->bsp_var_list[combuf->bsp_var_counter][coredata.pid] =
        (void*)variable;

    coredata.var_pushed = 1;
}

void EXT_MEM_TEXT bsp_pop_reg(const void* variable)
{
    ebsp_message(err_pop_reg_not_implemented);
    return;
}

void EXT_MEM_TEXT bsp_put(int pid, const void *src, void *dst, int offset, int nbytes)
{
    // Check if we can store the request
    if (coredata.request_counter >= MAX_DATA_REQUESTS)
        return ebsp_message(err_put_overflow);

    // Find remote address
    void* dst_remote = _get_remote_addr(pid, dst, offset);
    if (!dst_remote) return;

    // Check if we can store the payload
    // A mutex is needed for this.
    // While holding the mutex this core checks if it can store
    // the payload and if so, updates the buffer
    // Note that the mutex is NOT held while writing the payload itself
    // A possible error message is given after unlocking
    unsigned int payload_offset;

    e_mutex_lock(0, 0, &coredata.payload_mutex);

    payload_offset = combuf->data_payloads.buffer_size;

    if (payload_offset + nbytes > MAX_PAYLOAD_SIZE)
        payload_offset = -1;
    else
        combuf->data_payloads.buffer_size += nbytes;

    e_mutex_unlock(0, 0, &coredata.payload_mutex);

    if (payload_offset == -1)
        return ebsp_message(err_put_overflow2);

    // We are now ready to save the request and payload
    void* payload_ptr = &combuf->data_payloads.buf[payload_offset];

    // TODO(Tom)
    // Measure if e_dma_copy is faster here for both request and payload

    // Save request
    uint32_t req_count = coredata.request_counter;
    ebsp_data_request* req = &combuf->data_requests[coredata.pid][req_count];
    req->src = payload_ptr;
    req->dst = dst_remote;
    req->nbytes = nbytes | DATA_PUT_BIT;
    coredata.request_counter = req_count + 1;

    // Save payload
    ebsp_memcpy(payload_ptr, src, nbytes);
}

void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes)
{
    void* dst_remote = _get_remote_addr(pid, dst, offset);
    if (!dst_remote) return;
    ebsp_memcpy(dst_remote, src, nbytes);
}

void EXT_MEM_TEXT bsp_get(int pid, const void *src, int offset, void *dst, int nbytes)
{
    if (coredata.request_counter >= MAX_DATA_REQUESTS)
        return ebsp_message(err_get_overflow);
    const void* src_remote = _get_remote_addr(pid, src, offset);
    if (!src_remote) return;

    uint32_t req_count = coredata.request_counter;
    ebsp_data_request* req = &combuf->data_requests[coredata.pid][req_count];
    req->src = src_remote;
    req->dst = dst;
    req->nbytes = nbytes;
    coredata.request_counter = req_count + 1;
}

void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes)
{
    const void* src_remote = _get_remote_addr(pid, src, offset);
    if (!src_remote) return;
    ebsp_memcpy(dst, src_remote, nbytes);
}

void* ebsp_get_raw_address(int pid, const void* variable)
{
    return _get_remote_addr(pid, variable, 0);
}
