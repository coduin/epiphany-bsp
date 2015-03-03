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

#define DEBUG

#define _NPROCS 16

#define MAX_NAME_SIZE 30

// Every variable that is registered with bsp_push_reg
// gives 16 addresses (the locations on the different cores).
// An address takes 4 bytes, and MAX_N_REGISTER is the maximum
// amount of variables that can be registered so in total we need
// NCORES * MAX_N_REGISTER * 4 bytes to save all this data
#define MAX_N_REGISTER 40

// ebsp_core_data holds local bsp variables for the epiphany cores
// Every core has a copy in its local space
typedef struct {
    int                     pid;
    int                     nprocs;
    volatile int            syncstate; // ARM core will set this, epiphany will poll this
    volatile int            msgflag;
    unsigned int            last_timer_value;
    float                   time_passed; // not walltime but epiphany clock time
    void*                   registermap[MAX_N_REGISTER*_NPROCS];
} ebsp_core_data;

typedef struct {
    char msg[128];
} ebsp_message_buf;

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
typedef struct {
    int                 syncstate[_NPROCS];
    void*               pushregloc[_NPROCS];
    int                 msgflag[_NPROCS];
    ebsp_message_buf    message[_NPROCS];
    ebsp_core_data*     coredata[_NPROCS];
    float               remotetimer;
} ebsp_comm_buf;

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

// Clockspeed of Epiphany in cycles/second
// This was 'measured' by comparing with ARM wall-time measurements
// resulting in roughly 600 Mhz
#define CLOCKSPEED 600000000.0f
