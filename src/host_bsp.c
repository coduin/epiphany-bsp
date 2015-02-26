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
#include <stddef.h> //for offsetof

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

int write_coredata(int pid, void* src, off_t offset, int size)
{
    return co_write(pid, src, (off_t)state.comm_buf.coredata[pid] + offset, size);
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
    //TODO
    // When one of the functions fails half-way in bsp_begin
    // Then the functions that DID succeed should be undone again
    // So at e_load_group failure it cleanup the e_open result

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

    // Allocate communication buffer
    if (e_alloc(&state.emem, COMMBUF_OFFSET, sizeof(ebsp_comm_buf)) != E_OK)
    {
        fprintf(stderr, "ERROR: e_alloc failed in bspbegin.\n");
        return 0;
    }

    // Set initial values of buffer: zeroes and nprocs
    memset(&state.comm_buf, 0, sizeof(ebsp_comm_buf));
    state.comm_buf.nprocs = state.nprocs;

    // Write to epiphany
    if (e_write(&state.emem, 0, 0, 0, &state.comm_buf,
                sizeof(ebsp_comm_buf)) != sizeof(ebsp_comm_buf))
    {
        fprintf(stderr, "ERROR: e_write ebsp_comm_buf failed in bsp_begin.\n");
        return 0;
    }

    return 1;
}

int ebsp_spmd()
{   
    // Start the program
    // The program will block on bsp_begin
    // in state STATE_INIT
    // untill we send a STATE_CONTINUE
    // Before that, we have to fill its buffers
    // with data like timers
    if (e_start_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: e_start_group() failed.\n");
        return 0;
    }

    int i, j;
    int cores_initialized = 0;
    while (cores_initialized != state.nprocs)
    {
        usleep(1000); //0.01 seconds

        // Read the full communication buffer
        if (e_read(&state.emem, 0, 0, 0, &state.comm_buf,
                    sizeof(ebsp_comm_buf)) != sizeof(ebsp_comm_buf))
        {
            fprintf(stderr, "ERROR: e_read ebsp_comm_buf failed in ebsp_spmd.\n");
            return 0;
        }

        // Check every core
        for(i = 0; i < state.nprocs; ++i)
            if (state.comm_buf.syncstate[i] == STATE_INIT)
                ++cores_initialized;
    }

    //All cores are waiting.
    //We can now send data and 'start' them
    clock_t start=clock(); 
    clock_t end=clock(); 
    float arm_timer;
    arm_timer = (float)(end-start)/ARM_CLOCKSPEED;
    for(i = 0; i < state.nprocs; ++i)
    {
        //Core i has written its coredata address
        //Now we can initialize it
        write_coredata(i, &arm_timer, offsetof(ebsp_coredata, remotetimer), sizeof(int));
        //All data is initialized. Send start signal
        int flag = STATE_CONTINUE;
        write_coredata(i, &flag, offsetof(ebsp_coredata, syncstate), sizeof(int));
    }

    int total_syncs      = 0;
    int run_counter      = 0;
    int sync_counter     = 0;
    int finish_counter   = 0; 
    int continue_counter = 0;

    int iter = 0;

    while (finish_counter != state.nprocs) {
        //Write timer
        end=clock(); 
        arm_timer = (float)(end-start)/ARM_CLOCKSPEED;
        for(i = 0; i < state.nprocs; i++)
            write_coredata(i, &arm_timer, offsetof(ebsp_coredata, remotetimer), sizeof(int));

        // Read the full communication buffer
        // Including pushreg states and so on
        if (e_read(&state.emem, 0, 0, 0, &state.comm_buf,
                    sizeof(ebsp_comm_buf)) != sizeof(ebsp_comm_buf))
        {
            fprintf(stderr, "ERROR: e_read ebsp_comm_buf failed in ebsp_spmd.\n");
            return 0;
        }

        //Check sync states
        run_counter      = 0;
        sync_counter     = 0;
        finish_counter   = 0;
        continue_counter = 0;
        for (i = 0; i < state.nprocs; i++) {
            switch (state.comm_buf.syncstate[i]){
                case STATE_RUN:      run_counter++; break;
                case STATE_SYNC:     sync_counter++; break;
                case STATE_FINISH:   finish_counter++; break;
                case STATE_CONTINUE: continue_counter++; break;
                default: fprintf(stderr, "ERROR: syncstate[%d] = %d.\n",
                                 i, state.comm_buf.syncstate[i]);
                         break;
            }
            if (state.comm_buf.msgflag[i])
            {
                printf("$%02d: %s\n", i, state.comm_buf.message[i].msg);
                //Reset flag so we do not read it again
                state.comm_buf.msgflag[i] = 0;
                //Write the int to the external comm_buf
                e_write(&state.emem, 0, 0, offsetof(ebsp_comm_buf, msgflag[i]),
                        &state.comm_buf.msgflag[i], sizeof(int));
                //Signal epiphany core that message was read
                //so that it can (possibly) output the next message
                write_coredata(i, &state.comm_buf.msgflag[i],
                        offsetof(ebsp_coredata, msgflag), sizeof(int));
            }
        }

#ifdef DEBUG
        if (iter % 1000 == 0) {
            printf("Current time: %E seconds\n", arm_timer);
            printf("run %02d - sync %02d - finish %02d - continue %02d\n",
                    run_counter, sync_counter, finish_counter,
                    continue_counter);
        }
        ++iter;
#endif

        if (sync_counter == state.nprocs) {
            ++total_syncs;
#ifdef DEBUG
            //This part of the sync (host side)
            //usually does not crash so only one
            //line of debug output is needed here
            printf("(BSP) DEBUG: Sync %d\n", total_syncs);
#endif
            _host_sync();

            //First reset the comm_buf
            for(i = 0; i < state.nprocs; i++)
                state.comm_buf.syncstate[i] = STATE_CONTINUE;
            e_write(&state.emem, 0, 0, offsetof(ebsp_comm_buf, syncstate),
                    &state.comm_buf.syncstate, _NPROCS*sizeof(int));
            //Now write it to all cores to continue their execution
            for(i = 0; i < state.nprocs; i++)
                write_coredata(i, &state.comm_buf.syncstate[i],
                        offsetof(ebsp_coredata, syncstate), sizeof(int));
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

    e_free(&state.emem);

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

    // Check if core 0 did a push_reg
    if (state.comm_buf.pushregloc[0] != NULL)
    {
#ifdef DEBUG
        printf("(BSP) DEBUG: bsp_push_reg occurred. addr = %p\n",
                state.comm_buf.pushregloc[0]);
#endif

        if (state.num_vars_registered >= MAX_N_REGISTER)
        {
            fprintf(stderr, "ERROR: Trying to register more than %d variables.\n",
                    MAX_N_REGISTER);
        }
        else
        {
            // Broadcast registermap_buffer to registermap 
            for(i = 0; i < state.nprocs; ++i) {
                write_coredata(i, state.comm_buf.pushregloc,
                        (off_t)(offsetof(ebsp_coredata, registermap) +
                            sizeof(void*) * state.num_vars_registered * state.nprocs),
                        sizeof(void*) * state.nprocs);
            }

            state.num_vars_registered++;
#ifdef DEBUG
            printf("(BSP) DEBUG: New variables registered: %i/%i\n",
                    state.num_vars_registered,MAX_N_REGISTER);
#endif
        }

        // Reset pushregloc to zero
        for(i = 0; i < state.nprocs; ++i)
            state.comm_buf.pushregloc[i] = 0;
        if (e_write(&state.emem, 0, 0, offsetof(ebsp_comm_buf, pushregloc),
                    &state.comm_buf.pushregloc, _NPROCS*sizeof(void*)) != _NPROCS*sizeof(void*))
        {
            fprintf(stderr, "ERROR: e_write failed in _host_sync.\n");
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

