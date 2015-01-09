#pragma once

#define DEBUG

#define MAX_NAME_SIZE 30

#define REGISTERMAP_BUFFER_SHM_NAME "rmbshm"

#define MAX_N_REGISTER 10
#define REGISTERMAP_ADDRESS 0x7050
#define SYNC_STATE_ADDRESS 0x7200
#define NPROCS_LOC_ADDRESS 0x7210

#define CORE_ID _pid
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col

#define STATE_RUN 0
#define STATE_SYNC 1
#define STATE_CONTINUE 2
#define STATE_FINISH 3

#define NCORES (e_group_config.group_rows*e_group_config.group_cols)
#define MAX_NCORES 64
