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

#define MAX_NAME_SIZE 30

// Every variable that is registered with bsp_push_reg
// gives 16 addresses (the locations on the different cores).
// An address takes 4 bytes, and MAX_N_REGISTER is the maximum
// amount of variables that can be registered so in total we need
// NCORES * MAX_N_REGISTER * 4 bytes
// to save all this data
#define MAX_N_REGISTER          40

// ebsp_core_data holds local bsp variables for the epiphany cores
// Every core has a copy in its local space
typedef struct {
    int                     pid;
    int                     nprocs;
    volatile int            syncstate; //ARM core will set this, epiphany will poll this
    volatile int            msgflag;
    float                   remotetimer;
    unsigned int            initial_time;
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
} ebsp_comm_buf;

// ARM will e_alloc COMMBUF_OFFSET
// This same block will appear at COMMBUF_EADDR on epiphany
#define COMMBUF_OFFSET 0x01000000
#define COMMBUF_EADDR  0x8f000000

// Possible values for syncstate
// They start at 1 so that 0 means that the variable was not initialized
#define STATE_RUN       1
#define STATE_SYNC      2
#define STATE_CONTINUE  3
#define STATE_FINISH    4
#define STATE_INIT      5

#define NCORES (e_group_config.group_rows*e_group_config.group_cols)
#define MAX_NCORES 64

// Clockspeed of Epiphany in cycles/second
#define CLOCKSPEED 800000000.
#define ARM_CLOCKSPEED 20000.
