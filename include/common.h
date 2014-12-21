#pragma once
#define DEBUG
#define MAX_N_REGISTER 10
#define MAX_NAME_SIZE 30
const char registermap_buffer_shmname[] = "rmbshm";
#define REGISTERMAP_ADRESS 0x7050 //In local memory
#define CORE_ID e_group_config.core_row*e_group_config.group_cols+e_group_config.core_col
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col
#define NCORES e_group_config.group_rows*e_group_config.group_cols
