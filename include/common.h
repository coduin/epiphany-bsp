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

#define MAX_NAME_SIZE 30

#define SHM_NAME            "bspshm"
#define SHM_SIZE_PER_CORE   256

#define SHM_OFFSET_REGISTER 0
#define SHM_OFFSET_SYNC     4
#define SHM_OFFSET_MSG_FLAG 8
#define SHM_OFFSET_MSG_BUF  12
#define SHM_MESSAGE_SIZE    (SHM_SIZE_PER_CORE - SHM_OFFSET_MSG_BUF)
//SHM_OFFSET_MSG_BUF should always be the last part of the buffer

#define MAX_N_REGISTER 100
#define REGISTERMAP_ADDRESS 0x6150
#define SYNC_STATE_ADDRESS 0x6100
#define NPROCS_LOC_ADDRESS 0x6050
#define REMOTE_TIMER_ADDRESS 0x6040
#define LOC_BAR_ARRAY 0x6200
#define LOC_BAR_TGT_ARRAY 0x6300

#define CORE_ID _pid
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col

#define STATE_RUN 0
#define STATE_SYNC 1
#define STATE_CONTINUE 2
#define STATE_FINISH 3

#define NCORES (e_group_config.group_rows*e_group_config.group_cols)
#define MAX_NCORES 64

#define _NPROCS 16
//clockspeed of Epiphany in cycles/second
#define CLOCKSPEED 800000000.
#define ARM_CLOCKSPEED 20000.
