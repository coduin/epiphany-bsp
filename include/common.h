/*
File: common.h

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
#include <stdint.h>

// #define DEBUG

#define _NPROCS 16

// Every variable that is registered with bsp_push_reg
// gives 16 addresses (the locations on the different cores).
// An address takes 4 bytes, and MAX_BSP_VARS is the maximum
// amount of variables that can be registered so in total we need
// NCORES * MAX_BSP_VARS * 4 bytes to save all this data
#define MAX_BSP_VARS 64

// The maximum amount of buffered put/get operations each
// core is allowed to do per sync step
#define MAX_DATA_REQUESTS 128

// Maximum send operations for all cores together per sync step
#define MAX_MESSAGES      256

// The maximum amount of payload data for bsp_put and bsp_send operations
// This is shared amongst all cores!
#define MAX_PAYLOAD_SIZE (16*0x8000)

// The size of the buffers used for streaming (in bytes)
#define IN_CHUNK_SIZE 128
#define OUT_CHUNK_SIZE 64

// See ebsp_data_request::nbytes
#define DATA_PUT_BIT    (1<<31)

// Structures that are shared between ARM and epiphany
// need to use the same alignment
// By default, the epiphany compiler will align structs
// to 8-byte and the ARM compiler will align to 4-byte.
// Since all contents of the structs are 4-byte, we can
// safely use 4-byte packing without losing speed
#pragma pack(push, 4)

// Every bsp_put or bsp_get call results in an ebsp_data_request
// Additionally, bsp_put calls write to the ebsp_payload_buffer
typedef struct {
    // Both src and dst have the remote alias and offset included if applicable
    // so a memcpy can be used directly
    const void* src;
    void*       dst;

    // The highest bit of nbytes is used to indicate whether this is a
    // put or a get request. 0 means get, 1 means put
    int         nbytes;
} ebsp_data_request;

// bsp_put calls need to save the data payload
// Instead of having a separate buffer for each core there is one large
// buffer used for all cores together. This is because there are many
// applications that require a single core sending huge amounts of data
// while other cores send nothing. Since all cores access the same
// buffer there is a payload_mutex to ensure correctness
typedef struct {
    unsigned int    buffer_size;  // buffer used so far
    char            buf[MAX_PAYLOAD_SIZE];
} ebsp_payload_buffer;

// The pid in this struct is only needed in the current implementation
// where there is one large message queue for all cores instead
// of a single queue per core. This might change soon
typedef struct {
    int     pid;
    void*   tag;  // saved in same buffer as payload
    void*   payload;
    int     nbytes;  // payload bytes
} ebsp_message_header;

typedef struct {
    unsigned int        count;  // total messages so far
    ebsp_message_header message[MAX_MESSAGES];
} ebsp_message_queue;

// ebsp_comm_buf is a struct for epiphany <-> ARM communication
// It is located in external memory. For more info see
// https://github.com/buurlage-wits/epiphany-bsp/wiki/Memory-on-the-parallella

typedef struct
{
    // Epiphany --> ARM communication
    int8_t              syncstate[_NPROCS];
    int8_t*             syncstate_ptr;  // Location on epiphany core
    volatile int8_t     msgflag;  // 0: no msg. 1+pid: msg
    char                msgbuf[128];  // shared by all cores (mutexed)

    // ARM --> Epiphany
    float               remotetimer;
    int32_t             nprocs;
    int32_t             tagsize;  // Only for initial and final messages

    // Epiphany <--> Epiphany
    void*               bsp_var_list[MAX_BSP_VARS][_NPROCS];
    uint32_t            bsp_var_counter;
    ebsp_data_request   data_requests[_NPROCS][MAX_DATA_REQUESTS];
    ebsp_message_queue  message_queue[2];
    ebsp_payload_buffer data_payloads;  // used for put/get/send
} ebsp_comm_buf;

// Right after comm_buf there is the memory used for mallocs
// all the way till the end of external memory

#pragma pack(pop)

// For info on these addresses, see
// https://github.com/buurlage-wits/epiphany-bsp/wiki/Memory-on-the-parallella
#define SHARED_MEM     0x8e000000
#define SHARED_MEM_END 0x90000000
#define COMMBUF_OFFSET 0x01800000
#define COMMBUF_EADDR  (SHARED_MEM + COMMBUF_OFFSET)

// Possible values for syncstate
// They start at 1 so that 0 means that the variable was not initialized
#define STATE_UNDEFINED 0
#define STATE_RUN       1
#define STATE_SYNC      2
#define STATE_CONTINUE  3
#define STATE_FINISH    4
#define STATE_INIT      5
#define STATE_EREADY    6
#define STATE_ABORT     7

// Clockspeed of Epiphany in cycles/second
// This was 'measured' by comparing with ARM wall-time measurements
// resulting in roughly 600 Mhz
#define CLOCKSPEED 600000000.0f

