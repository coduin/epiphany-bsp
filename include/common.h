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

//#define DEBUG

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
#define MAX_PAYLOAD_SIZE 0x8000

// See ebsp_data_request::nbytes
#define DATA_PUT_BIT    (1<<31)

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

// Structures that are shared between ARM and epiphany
// need to use the same alignment: use maximum packing
#pragma pack(push,1)

// bsp_put calls need to save the data payload
// Instead of having a separate buffer for each core there is one large
// buffer used for all cores together. This is because there are many
// applications that require a single core sending huge amounts of data
// while other cores send nothing. Since all cores access the same
// buffer there is a payload_mutex to ensure correctness
typedef struct {
    unsigned int    buffer_size; // buffer used so far
    char            buf[MAX_PAYLOAD_SIZE];
} ebsp_payload_buffer;

// The pid in this struct is only needed in the current implementation
// where there is one large message queue for all cores instead
// of a single queue per core. This might change soon
typedef struct {
    int     pid;
    void*   tag; // saved in same buffer as payload
    void*   payload;
    int     nbytes; // payload bytes
} ebsp_message_header;

typedef struct {
    unsigned int        count; // total messages so far
    ebsp_message_header message[MAX_MESSAGES];
} ebsp_message_queue;

// ebsp_comm_buf is a struct for epiphany -> ARM communication
// It is located in "external memory", meaning on the epiphany device
// but not in one of the cores internal memory
// The epiphany cores write a value here once
// and the arm core polls these values
//
// Note that we do NOT want:
//      struct{
//         int syncstate;
//         int msgflag;
//         ...;
//      } coreinfo;
//
//      struct{
//         coreinfo cores[16];
//      } ebsp_comm_buf;
//
// This is not efficient because by interlacing
// the data for different cores (all syncstate flags are
// in one memory chunk, all msgflags in the next) we can read
// all syncstate flags in ONE e_read call instead of 16

typedef struct
{
    // Epiphany --> ARM communication
    int                 syncstate[_NPROCS];
    int*                syncstate_ptr; // So that ARM knows where syncstate is
    volatile int        msgflag; // 0: no msg. 1+pid: msg
    char                msgbuf[128]; // shared by all cores (mutexed)

    // ARM --> Epiphany
    float               remotetimer;
    int                 nprocs;
    int                 initial_tagsize;

    // Epiphany <--> Epiphany
    void*               bsp_var_list[MAX_BSP_VARS][_NPROCS];
    unsigned int        bsp_var_counter;
    ebsp_data_request   data_requests[_NPROCS][MAX_DATA_REQUESTS];
    ebsp_message_queue  message_queue[2];
    ebsp_payload_buffer data_payloads; // used for put/get/send
} ebsp_comm_buf;

#pragma pop(pack)

// The following info can be found in the linker scripts.
//
// The shared memory (as seen from the epiphany) is located at
// [0x8e000000, 0x90000000[ = [0x8e000000, 0x8fffffff]
// Total size 0x02000000 = 32MB
//
// If one uses the fast.ldf linker script the shared memory is split as:
// [0x00000000, 0x01000000[ (16 MB)
//     - libc code,data,stack
// [0x01000000, 0x02000000[ (16 MB)
//     - "shared_dram" 8MB used by e_shm_xxx functions
//     -   "heap_dram" 8MB, or 512K heap per core
//
// One can store variables in these parts using
// int myint SECTION("shared_dram");
// int myint SECTION("heap_dram");
//
// To use the shared_dram properly one can use the e_shm_xxx
// functions which properly allocate memory in "shared_dram"
// However, testing shows that some libc functions (like printf with %f) can
// overwrite memory in "shared_dram" so therefore we use "heap_dram" for
// the communication buffer instead of the e_shm_xxx functions
// 
// On the ARM side, we use e_alloc which allows us to access all shared memory
//
// Andreas notes that the shared dram is slow compared to internal core memory:
//     If you are reading (loading) directly out of global DDR,
//     it takes hundreds of clock cycles to do every read,
//     so it's going to be very slow. Every read in the program is blocking,
//     and the read has to first go through
//     the mesh-->off chip elink-->FPGA logic-->AXI bus on Zynq
//     -->memory controller-->DRAM (and back).
// http://forums.parallella.org/viewtopic.php?f=23&t=1660
#define SHARED_MEM     0x8e000000
#define COMMBUF_OFFSET 0x01800000
#define COMMBUF_EADDR  (SHARED_MEM + COMMBUF_OFFSET)

// Possible values for syncstate
// They start at 1 so that 0 means that the variable was not initialized
#define STATE_RUN       1
#define STATE_SYNC      2
#define STATE_CONTINUE  3
#define STATE_FINISH    4
#define STATE_INIT      5
#define STATE_EREADY    6

// Clockspeed of Epiphany in cycles/second
// This was 'measured' by comparing with ARM wall-time measurements
// resulting in roughly 600 Mhz
#define CLOCKSPEED 600000000.0f

