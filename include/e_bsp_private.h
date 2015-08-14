/*
File: e_bsp_private.h

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

#pragma once
#include "e_bsp.h"
#include "common.h"
#include <e-lib.h>
#include <stdint.h>

// Use this define to place functions or variables in external memory
// TEXT is for functions and normal variables
// RO is for read only globals
#define EXT_MEM_TEXT __attribute__((section("EBSP_TEXT")))
#define EXT_MEM_RO   __attribute__((section("EBSP_RO")))

//
// All internal bsp variables for this core
// 8-bit variables (mutexes) are grouped together
// to avoid unnecesary padding
//
typedef struct {
    // ARM core will set this, epiphany will poll this
    volatile int8_t syncstate;

    int32_t         pid;
    int32_t         nprocs;

    // time_passed is epiphany cpu time (so not walltime) in seconds
    float           time_passed;

    // counter for ebsp_combuf::data_requests[pid]
    uint32_t        request_counter;

    // message_index is an index into an epiphany<->epiphany queue and
    // when it reached the end, it is an index into the arm->epiphany queue
    uint32_t        tagsize;
    uint32_t        tagsize_next;  // next superstep
    uint32_t        queue_index;
    uint32_t        message_index;

    // bsp_sync barrier
    volatile e_barrier_t sync_barrier[_NPROCS];
    e_barrier_t*    sync_barrier_tgt[_NPROCS];

    // if this core has done a bsp_push_reg
    int8_t          var_pushed;

    // Mutex is used for message_queue (send) and data_payloads (put)
    e_mutex_t       payload_mutex;

    // Mutex for ebsp_message
    e_mutex_t       ebsp_message_mutex;

    // Mutex for ebsp_ext_malloc (internal malloc does not have mutex)
    e_mutex_t       malloc_mutex;

    // Base address of malloc table for internal malloc
    void*           local_malloc_base;

    // Pointer to the next input chunk in exmem
    void*           exmem_next_in_chunk;

    // Pointer to the current output chunk in exmem
    void*           exmem_current_out_chunk;

    // Pointer to current input buffer (in local memory)
    void*           buffer_in_current;

    // Pointer to current output buffer (in local memory)
    void*           buffer_out_current;

    // Pointer to next input buffer (in local memory)
    void*           buffer_in_next;

    // Pointer to previous output buffer (in local memory)
    void*           buffer_out_previous;

    // DMA descriptor needed for channel E_DMA_0
    e_dma_desc_t    _dma_copy_descriptor_0;

    // DMA descriptor needed for channel E_DMA_1
    e_dma_desc_t    _dma_copy_descriptor_1;
} ebsp_core_data;

extern ebsp_core_data coredata;

// The define is faster; it saves a pointer lookup
#define comm_buf ((ebsp_combuf*)E_COMBUF_ADDR)
// ebsp_combuf * const comm_buf = (ebsp_combuf*)E_COMBUF_ADDR;

void _init_local_malloc();

// Wrapper for usage of DMA engine
int ebsp_dma_copy_parallel(e_dma_id_t chan, void *dst, void *src, size_t n);

