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
#include <time.h>

// Global state
bsp_state_t state;
int bsp_initialized = 0;;

void _host_sync();
void _get_p_coords(int pid, int* row, int* col);
bsp_state_t* _get_state();

int co_write(int pid, void* src, off_t dst, int size)
{
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    if (e_write(&state.dev,
            prow, pcol,
            dst, src, size) != size)
    {
        fprintf(stderr, "ERROR: e_write(dev,%d,%d,%p,%p,%d) failed in co_write.\n",
                prow, pcol, (void*)dst, (void*)src, size);
        return 0;
    }
    return 1;
}

int co_read(int pid, off_t src, void* dst, int size)
{
    int prow, pcol;
    _get_p_coords(pid, &prow, &pcol);
    if (e_read(&state.dev,
           prow, pcol,
           src, dst, size) != size)
    {
        fprintf(stderr, "ERROR: e_read(dev,%d,%d,%p,%p,%d) failed in co_read.\n",
                prow, pcol, (void*)src, (void*)dst, size);
        return 0;
    }
    return 1;
}

int bsp_init(const char* _e_name,
        int argc,
        char **argv)
{
    if (bsp_initialized) {
        fprintf(stderr, "ERROR: bsp_init called when already initialized.\n");
        return 0;
    }

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

    bsp_initialized = 1;

    return 1;
}

int bsp_begin(int nprocs)
{
    if (nprocs < 1 || nprocs > 99 || nprocs > MAX_NCORES) {
        fprintf(stderr, "ERROR: nprocs = %d.\n", nprocs);
        return 0;
    }

    //TODO: non-rectangle
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
            if (e_write(&state.dev, i, j, (off_t)NPROCS_LOC_ADDRESS,
                        &state.nprocs, sizeof(int)) != sizeof(int)) {
                fprintf(stderr, "ERROR: e_write(dev,%d,%d,..) failed in bsp_begin.\n",i,j);
                return 0;
            }
        }
    }

    // Allocate shared memory buffers
    char rm_name[10];
    for(i = 0; i < state.nprocs; ++i) {
        sprintf(rm_name, "%s_%i", REGISTERMAP_BUFFER_SHM_NAME, i);
        if (e_shm_alloc(&state.registermap_buffer[i],
                    rm_name, sizeof(void*)) != E_OK)
        {
            fprintf(stderr, "ERROR: e_shm_alloc failed on %s.\n", rm_name);
            return 0; 
        }
        sprintf(rm_name, "%s_%i", SYNCFLAG_SHM_NAME, i);
        if (e_shm_alloc(&state.syncflag_buffer[i], rm_name, sizeof(int)) != E_OK)
        {
            fprintf(stderr, "ERROR: e_shm_alloc failed on %s.\n", rm_name);
            return 0;
        }
    }

    //Set shared memory buffers to zero
    int buf=0;
    for(i = 0; i < state.nprocs; ++i)  {
        if (e_write(&state.registermap_buffer[i], 0, 0, (off_t) 0,
                    (void*)&buf, sizeof(void*)) != sizeof(void*)) {
            fprintf(stderr, "ERROR: e_write(registemap_buffer[%d],..) failed in bsp_begin.\n",i);
            return 0;
        }
        if (e_write(&state.syncflag_buffer[i], 0, 0, 0,
                    &buf, sizeof(int)) != sizeof(int))
        {
            fprintf(stderr, "ERROR: e_write(syncflag_buffer[%d],..) failed in bsp_begin.\n",i);
            return 0;
        }
    }

    return 1;
}

int ebsp_spmd()
{   
    int i = 0;
    int j = 0;

    clock_t start=clock(); 
    clock_t end=clock(); 
    float arm_timer;
    arm_timer = (float)(end-start)/ARM_CLOCKSPEED;
    for(i = 0; i < state.nprocs; i++) {
        co_write(i, &arm_timer, (off_t)REMOTE_TIMER_ADDRESS, sizeof(int));
    }
    
    // Start the program
    if (e_start_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: e_start_group() failed.\n");
        return 0;
    }
    // sleep for 0.01 seconds
    usleep(1000);


    int state_flag = 0;

    int sync_counter     = 0;
    int finish_counter   = 0; 
    int continue_counter = 0;

    int iter = 0;

    while (finish_counter != state.nprocs) {
        end=clock(); 
        arm_timer = (float)(end-start)/ARM_CLOCKSPEED;
        for(i = 0; i < state.nprocs; i++) {
            co_write(i, &arm_timer, (off_t)REMOTE_TIMER_ADDRESS, sizeof(int));
        }
        sync_counter     = 0;
        finish_counter   = 0;
        continue_counter = 0;
        for (i = 0; i < state.nprocs; i++) {
            state_flag = 0;
            //co_read(i, (off_t)SYNC_STATE_ADDRESS, &state_flag, sizeof(int));
            if (e_read(&state.syncflag_buffer[i], 0, 0, 0,
                        &state_flag, sizeof(int)) != sizeof(int))
                fprintf(stderr, "ERROR: shm syncflag read failed.\n");

            if (state_flag == STATE_SYNC    ) sync_counter++;
            if (state_flag == STATE_FINISH  ) finish_counter++;
            if (state_flag == STATE_CONTINUE) continue_counter++;
        }

#ifdef DEBUG
        if (iter % 100 == 0) {
            printf("Current time: %E seconds\n", arm_timer);
            printf("sync \t finish \t continue \t 16th stateflag \n");
            printf("%i \t %i \t\t %i \t\t %i\n", 
                    sync_counter,
                    finish_counter,
                    continue_counter,
                    state_flag);
        }
        ++iter;
#endif

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
                //shared mem, to reset flag
                if (e_write(&state.syncflag_buffer[i], 0, 0, 0,
                            &state_flag, sizeof(int)) != sizeof(int))
                    fprintf(stderr, "ERROR: e_write(syncflag_buffer[%d],..) failed in ebsp_spmd.\n",i);
                //to core, will cause execution to continue
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
    if (!bsp_initialized) {
        fprintf(stderr, "ERROR: bsp_end called when bsp was not initialized.\n");
        return 0;
    }
    
    //Release shared memory
    int i;
    char rm_name[10];
    for(i = 0; i < state.nprocs; ++i) {
        sprintf(rm_name, "%s_%i", REGISTERMAP_BUFFER_SHM_NAME, i);
        if(E_OK != e_shm_release(rm_name))
            fprintf(stderr, "ERROR: e_shm_release(%s) failed.\n",rm_name);

        sprintf(rm_name, "%s_%i", SYNCFLAG_SHM_NAME, i);
        if(E_OK != e_shm_release(rm_name))
            fprintf(stderr, "ERROR: e_shm_release(%s) failed.\n",rm_name);
    }

    if(E_OK != e_finalize()) {
        fprintf(stderr, "ERROR: Could not finalize the Epiphany connection.\n");
        return 0;
    }

    free(state.e_name);
    memset(&state, 0, sizeof(state));
    bsp_initialized = 0;

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

    int i, j;
    int new_vars = 0;
#ifdef DEBUG
    printf("(BSP) DEBUG: _host_sync() ............................... \n");
#endif

    void* var_loc;
    if (e_read(&state.registermap_buffer[0], 0, 0, 0, &var_loc, sizeof(void*))
            != sizeof(void*)) {
        fprintf(stderr, "ERROR: in host_sync() could not read registermap_buffer[0]");
        return;
    }
        
#ifdef DEBUG
    printf("var_loc = 0x%x\n", (int)var_loc);
#endif

    if(var_loc != NULL) {
        new_vars = 1;
    } else {
        return;
    }

    //TODO: Do not malloc a constant size buffer on every sync
    //malloc/free on start/end of ebsp_spmd

    // Broadcast registermap_buffer to registermap 
    void** buf = (void**) malloc(sizeof(void*) * state.nprocs);
    for(i = 0; i < state.nprocs; ++i) {
        if (e_read(&state.registermap_buffer[i], 0, 0, (off_t)0, buf + i, sizeof(void*))
                != sizeof(void*)) {
            fprintf(stderr, "ERROR: in host_sync() could not read registermap_buffer[i]");
            //dont return, because of malloc
        }
    }
    for(i = 0; i < state.nprocs; ++i) {
        co_write(i, buf,
                (off_t)(REGISTERMAP_ADDRESS + state.num_vars_registered * state.nprocs), 
                sizeof(void*) * state.nprocs);
    }
    free(buf);

    state.num_vars_registered++;
#ifdef DEBUG
    printf("(BSP) DEBUG: New variables registered: %i/%i\n",state.num_vars_registered,MAX_N_REGISTER);
#endif

    // Reset registermap_buffer to zero
    void* buffer = 0;
    //FIXME; ee_mwrite_buf(): Address is out of bounds.
    for(i = 0; i < state.nprocs; ++i) {
        if (e_write(&state.registermap_buffer[i], 0, 0, (off_t)0,
                    (void*)&buffer, sizeof(void*)) != sizeof(void*)) {
            fprintf(stderr, "ERROR: in host_sync() could not write registermap_buffer[i]");
        }
    }
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
