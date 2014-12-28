#define DEBUG

#define MAX_NAME_SIZE 30

#define REGISTERMAP_BUFFER_SHM_NAME "rmbshm";

#define REGISTERMAP_ADDRESS 0x7050 //In local memory
#define MAX_N_REGISTER 10

#define CORE_ID (e_group_config.core_row*e_group_config.group_cols+e_group_config.core_col)
#define CORE_ROW e_group_config.core_row
#define CORE_COL e_group_config.core_col

#define NCORES (e_group_config.group_rows*e_group_config.group_cols)
