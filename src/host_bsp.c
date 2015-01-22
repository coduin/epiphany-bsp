/*
File: bsp_host.c

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

#include "host_bsp.h"
#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// Global state
bsp_state_t state;

void _host_sync();
void _get_p_coords(int pid, int* row, int* col);
bsp_state_t* _get_state();

void co_write(int pid, void* src, off_t dst, int size)
{
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    e_write(&state.dev,
            prow, pcol,
            dst, src, size);
}

void co_read(int pid, off_t src, void* dst, int size)
{
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    e_read(&state.dev,
           prow, pcol,
           src, dst, size);
}

int bsp_init(const char* _e_name,
        int argc,
        char **argv)
{
    // Initialize the Epiphany system for the working with the host application
    if(e_init(NULL) != E_OK) {
        fprintf(stderr, "ERROR: Could not initialize HAL data structures.\n");
        return 0;
    }

    // Reset the Epiphany system
    if(e_reset_system() != E_OK) {
        fprintf(stderr, "ERROR: Could not reset the Epiphany system.\n");
        return 0;
    }

    // Get information on the platform
    if(e_get_platform_info(&state.platform) != E_OK) {
        fprintf(stderr, "ERROR: Could not obtain platform information.\n");
        return 0;
    }

    // Obtain the number of processors from the platform information
    state.nprocs = state.platform.rows * state.platform.cols;

    // Copy the name to the state
    state.e_name = (char*)malloc(MAX_NAME_SIZE);
    strcpy(state.e_name, _e_name);

    return 1;
}

int bsp_begin(int nprocs)
{
    state.rows = (nprocs / state.platform.rows);
    state.cols = nprocs / (nprocs / state.platform.rows);

    printf("(BSP) INFO: Making a workgroup of size %i x %i\n",
            state.rows,
            state.cols);

    state.nprocs_used = nprocs;
    state.num_vars_registered = 0;

    // Open the workgroup
    if(e_open(&state.dev,
                0, 0,
                state.rows,
                state.cols) != E_OK)
    {
        fprintf(stderr, "ERROR: Could not open workgroup.\n");
        return 0;
    }

    if(e_reset_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: Could not reset workgroup.\n");
        return 0;
    }


    // Load the e-binary
    printf("(BSP) INFO: Loading: %s\n", state.e_name);
    if(e_load_group(state.e_name,
                &state.dev,
                0, 0,
                state.rows, state.cols, 
                E_FALSE) != E_OK)
    {
        fprintf(stderr, "ERROR: Could not load program in workgroup.\n");
        return 0;
    }

    // Write the nprocs to memory
    int i, j;
    for(i = 0; i < state.platform.rows; ++i) {
        for(j = 0; j < state.platform.cols; ++j) {
            e_write(&state.dev, i, j, (off_t)NPROCS_LOC_ADDRESS, &state.nprocs, sizeof(int));
        }
    }

    // Allocate registermap_buffers
    printf("DEBUG: Allocate registermap_buffers..\n");
    for(i = 0; i < state.nprocs; ++i) {
        char rm_name[10];
        strcpy(rm_name, REGISTERMAP_BUFFER_SHM_NAME);
        char id[3] = { 0 };
        sprintf(id, "_%i", i);
        strcat(rm_name, id);

        printf("DEBUG: Writing to registermap_buffer[%i], rm_name=%s\n",i, rm_name);
        if(e_shm_alloc(&state.registermap_buffer[i],
                    rm_name, sizeof(void*)) != E_OK) {
            fprintf(stderr, "ERROR: Could not allocate registermap_buffer %s.\n", rm_name);
            return 0; 
        }
    }

    //Set registermap_buffer to zero FIXME: IT ALWAYS REGISTERS
    int buf=0;
    printf("DEBUG: Setting registermap_buffer to zero..\n");
    for(i = 0; i < state.nprocs; ++i)  {
        e_write(&state.registermap_buffer[i], 0, 0, (off_t) 0, (void*)&buf, sizeof(void*));
    }

    printf("DEBUG: Registering DONE..\n");
    return 1;
}

int ebsp_spmd()
{
    // Start the program
    e_start_group(&state.dev);

    // sleep for 0.01 seconds
    usleep(1000);

    int i = 0;
    int j = 0;

    int state_flag = 0;

    int sync_counter     = 0;
    int finish_counter   = 0; 
    int continue_counter = 0;

    int iter = 0;

    while (finish_counter != state.nprocs) {
        sync_counter     = 0;
        finish_counter   = 0;
        continue_counter = 0;
        for (i = 0; i < state.nprocs; i++) {
            co_read(i, (off_t)SYNC_STATE_ADDRESS, &state_flag, sizeof(int));

            if (state_flag == STATE_SYNC    ) sync_counter++;
            if (state_flag == STATE_FINISH  ) finish_counter++;
            if (state_flag == STATE_CONTINUE) continue_counter++;
        }

        if (iter % 1000 == 0) {
            printf("sync \t finish \t continue \t 16th stateflag \n");
            printf("%i \t %i \t %i \t %i\n", 
                sync_counter,
                finish_counter,
                continue_counter,
                state_flag);
        }
        ++iter;

        if (sync_counter == state.nprocs) {
#ifdef DEBUG
            printf("(BSP) DEBUG: Syncing on host...\n");
#endif
            _host_sync();

#ifdef DEBUG
            printf("(BSP) DEBUG: Writing STATE_CONTINUE to processors...\n");
#endif
            state_flag = STATE_CONTINUE;
            for(i = 0; i < state.nprocs; i++) {
                co_write(i, &state_flag, (off_t)SYNC_STATE_ADDRESS, sizeof(int));
            }
#ifdef DEBUG
            printf("(BSP) DEBUG: Continuing...\n");
#endif
        }

        usleep(1000);
    }
    printf("(BSP) INFO: Program finished\n");

    return 1;
}

int bsp_end()
{

    printf("DEBUG: BSP_end..\n");
    // FIXME release all
    /* if(E_OK != e_shm_release(REGISTERMAP_BUFFER_SHM_NAME) ) {
        fprintf(stderr, "ERROR: Could not relese registermap_buffer\n");
        return 0;
    } */
    if(E_OK != e_finalize()) {
        fprintf(stderr, "ERROR: Could not finalize the Epiphany connection.\n");
        return 0;
    }
    return 1;
}

int bsp_nprocs()
{
    return state.nprocs;
}

//------------------
// Private functions
//------------------

// Memory
void _host_sync() {
    // TODO: Right now bsp_pop_reg is ignored
    // TODO: ALWAYS THINKS NEW REGISTRATION IS REQUIRED
    // Check if overwrite is necessary => this gives no problems

    int i, j;
    int new_vars = 0;
#ifdef DEBUG
    printf("(BSP) DEBUG: registermapbuffer contents: ");
#endif

    void* var_loc;
    e_read(&state.registermap_buffer[0], 0, 0, 0, &var_loc, sizeof(void*));
        
#ifdef DEBUG
    printf("0x%x\n", (int)var_loc);
#endif

    if(var_loc != NULL) {
        new_vars = 1;
    } else {
        return;
    }

    // Broadcast registermap_buffer to registermap 
    void** buf = (void**) malloc(sizeof(void*) * state.nprocs);
    for(i = 0; i < state.nprocs; ++i) {
        e_read(&state.registermap_buffer[i], 0, 0, (off_t)0, buf + i, sizeof(void*));
    }

    printf("DEBUG: Broadcastring registermap_buffer to registermap, write phase\n");
    for(i = 0; i < state.nprocs; ++i) {
        co_write(i, buf,
                (off_t)(REGISTERMAP_ADDRESS + state.num_vars_registered * state.nprocs), 
                sizeof(void*) * state.nprocs);
    }

    state.num_vars_registered++;
#ifdef DEBUG
    printf("(BSP) DEBUG: New variables registered: %i/%i\n",state.num_vars_registered,MAX_N_REGISTER);
#endif

    // Reset registermap_buffer
    printf("DEBUG: Resetting registermap_buffer\n");//FIXME; ee_mwrite_buf(): Address is out of bounds.
    void* buffer = calloc(sizeof(void*), state.nprocs);
    printf("buffer: %i\n",(int)buffer);
    for(i = 0; i < state.nprocs; ++i) {
        e_write(&state.registermap_buffer[i], 0, 0, (off_t)0, buffer, sizeof(void*));
    }

    printf("DEBUG: Freeing memory\n");
    free(buf);
    free(buffer);
    printf("DEBUG: _host_sync DONE\n");
}

void _get_p_coords(int pid, int* row, int* col)
{
    (*row) = pid / state.cols;
    (*col) = pid % state.cols;
}

bsp_state_t* _get_state()
{
    return &state;
}
