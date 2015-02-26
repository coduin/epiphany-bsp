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

//Every core has MAX_N_REGISTER
//Every register is sizeof(void*) = 4
//So NCORES*MAX_N_REGISTER*4 is required
//for registermap buffer
#define MAX_N_REGISTER          40

//struct for local eCore bsp variables
//every core has a copy at local core memory
//these used to be hardcoded at 0x6000
//but are now stored as a global variable in the binary
typedef struct {
    int                     _pid;
    int                     nprocs;
    volatile int            syncstate; //ARM core will set this, epiphany will poll this
    volatile int            msgflag;
    float                   remotetimer;
    unsigned int            _initial_time;
    void*                   registermap[MAX_N_REGISTER*_NPROCS];
    //volatile e_barrier_t    syncbarrier[_NPROCS]; //16 bytes
    //e_barrier_t*            syncbarrier_tgt[_NPROCS]; //16 pointers to the bytes on other cores
} ebsp_coredata;

//Buffer for ebsp_message
typedef struct {
    char msg[128];
} ebsp_message_buf;

//struct for eCore --> ARM communication
//This is located at "external memory"
//meaning on the epiphany side, but not
//in a local core's memory
//The epiphany cores write a value here once
//and the arm core polls these values
//
//Note that we do NOT want:
// struct{
//     int syncstate;
//     int msgflag;
//     ...;
//  } coreinfo;
//  struct{
//     coreinfo cores[16];
//  } ebsp_comm_buf;
//
//  This is not efficient because by interlacing
//  the data for different cores (all syncstate flags are
//  in one memory chunk, all msgflags in the next) we can read
//  all syncstate flags in ONE e_read call instead of 16
typedef struct {
    int                 syncstate[_NPROCS]; //epiphany cores will set these, ARM core will poll these
    void*               pushregloc[_NPROCS];
    int                 msgflag[_NPROCS];
    ebsp_message_buf    message[_NPROCS];
    ebsp_coredata*      coredata[_NPROCS];
    int                 nprocs;
} ebsp_comm_buf;

//ARM will e_alloc COMMBUF_OFFSET
#define COMMBUF_OFFSET 0x01000000
//This same block will appear at COMMBUF_EADDR
//on epiphany
#define COMMBUF_EADDR  0x8f000000

#define CORE_ID _pid
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col

//Do not start at 0 so that we can
//check if the variable has been
//initialized at all
#define STATE_RUN       1
#define STATE_SYNC      2
#define STATE_CONTINUE  3
#define STATE_FINISH    4
#define STATE_INIT      5

#define NCORES (e_group_config.group_rows*e_group_config.group_cols)
#define MAX_NCORES 64

//clockspeed of Epiphany in cycles/second
#define CLOCKSPEED 800000000.
#define ARM_CLOCKSPEED 20000.
