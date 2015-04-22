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

    // counter for ebsp_comm_buf::data_requests[pid]
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

    // Mutex for malloc C function
    e_mutex_t       malloc_mutex;
} ebsp_core_data;

extern ebsp_core_data coredata;

// The define is faster; it saves a pointer lookup
#define comm_buf ((ebsp_comm_buf*)COMMBUF_EADDR)
// ebsp_comm_buf * const comm_buf = (ebsp_comm_buf*)COMMBUF_EADDR;

void _init_malloc_state();

