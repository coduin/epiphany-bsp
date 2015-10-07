/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
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

#include "host_bsp_private.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <e-loader.h>

#define __USE_XOPEN2K
#include <unistd.h> // For the function 'access' in bsp_init

bsp_state_t state;

int bsp_initialized = 0;

int bsp_init(const char* _e_name, int argc, char** argv) {
    if (bsp_initialized) {
        fprintf(stderr, "ERROR: bsp_init called when already initialized.\n");
        return 0;
    }

    // Get the path to the application and append the epiphany executable name
    init_application_path();
    snprintf(state.e_fullpath, sizeof(state.e_fullpath), "%s%s",
             state.e_directory, _e_name);

    // Check if the file exists
    if (access(state.e_fullpath, R_OK) == -1) {
        fprintf(stderr, "ERROR: Could not find epiphany executable: %s\n",
                state.e_fullpath);
        return 0;
    }

    // Initialize the Epiphany system for the working with the host application
    if (e_init(NULL) != E_OK) {
        fprintf(stderr, "ERROR: Could not initialize HAL data structures.\n");
        return 0;
    }

    // Reset the Epiphany system
    if (e_reset_system() != E_OK) {
        fprintf(stderr, "ERROR: Could not reset the Epiphany system.\n");
        return 0;
    }

    // Get information on the platform
    if (e_get_platform_info(&state.platform) != E_OK) {
        fprintf(stderr, "ERROR: Could not obtain platform information.\n");
        return 0;
    }

    // Obtain the number of processors from the platform information
    state.nprocs = state.platform.rows * state.platform.cols;

    // Initialize buffering
    for (int p = 0; p < NPROCS; p++) {
        state.combuf.n_streams[p] = 0;
    }

    bsp_initialized = 1;

    return 1;
}

int bsp_begin(int nprocs) {
    // TODO(*)
    // When one of the functions fails half-way in bsp_begin
    // Then the functions that DID succeed should be undone again
    // So at e_load_group failure it cleanup the e_open result

    if (nprocs < 1 || nprocs > NPROCS) {
        fprintf(stderr, "ERROR: bsp_begin called with nprocs = %d.\n", nprocs);
        return 0;
    }

    // TODO(*) non-rectangle
    // state.rows = (nprocs / state.platform.rows);
    // state.cols = nprocs / (nprocs / state.platform.rows);
    state.rows = state.platform.rows;
    state.cols = state.platform.cols;

#ifdef DEBUG
    printf("(BSP) INFO: Making a workgroup of size %i x %i\n", state.rows,
           state.cols);
#endif

    state.nprocs_used = nprocs;
    state.num_vars_registered = 0;

    // Open the workgroup
    if (e_open(&state.dev, 0, 0, state.rows, state.cols) != E_OK) {
        fprintf(stderr, "ERROR: Could not open workgroup.\n");
        return 0;
    }

    if (e_reset_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: Could not reset workgroup.\n");
        return 0;
    }

// Load the e-binary
#ifdef DEBUG
    printf("(BSP) INFO: Loading: %s\n", state.e_fullpath);
#endif
    if (e_load_group(state.e_fullpath, &state.dev, 0, 0, state.rows, state.cols,
                     E_FALSE) != E_OK) {
        fprintf(stderr, "ERROR: Could not load program in workgroup.\n");
        return 0;
    }

    // e_alloc will mmap combuf and dynmem
    // The offset in external memory is equal to NEWLIB_SIZE
    if (e_alloc(&state.emem, NEWLIB_SIZE, COMBUF_SIZE + DYNMEM_SIZE) != E_OK) {
        fprintf(stderr, "ERROR: e_alloc failed in bspbegin.\n");
        return 0;
    }
    state.host_combuf_addr = state.emem.base;
    state.host_dynmem_addr = state.emem.base + COMBUF_SIZE;

    ebsp_malloc_init();

    // Set initial buffer to zero so that it can be filled by messages
    // before calling ebsp_spmd
    memset(&state.combuf, 0, sizeof(ebsp_combuf));

    return 1;
}

int ebsp_spmd() {

    // Write stream structs to combuf + extmem
    for (int p = 0; p < NPROCS; p++) {
        int nbytes = state.combuf.n_streams[p] * sizeof(ebsp_stream_descriptor);
        void* stream_descriptors = ebsp_ext_malloc(nbytes);
        memcpy(stream_descriptors, state.buffered_streams[p], nbytes);
        state.combuf.extmem_streams[p] = _arm_to_e_pointer(stream_descriptors);

        // TODO void*               extmem_current_out_chunk[NPROCS];
        // TODO int                 out_buffer_size[NPROCS];
    }

    // Write communication buffer containing nprocs,
    // messages and tagsize
    state.combuf.nprocs = state.nprocs_used;
    for (int i = 0; i < state.nprocs; ++i)
        state.combuf.syncstate[i] = STATE_INIT;
    if (!_write_extmem(&state.combuf, 0, sizeof(ebsp_combuf))) {
        fprintf(stderr, "ERROR: initial extmem write failed in ebsp_spmd.\n");
        return 0;
    }

    // Starting time
    clock_gettime(CLOCK_MONOTONIC, &state.ts_start);
    _update_remote_timer();

    // Start the program
    // Only in DEBUG mode:
    // The program will block on bsp_begin in state STATE_EREADY
    // untill we send a STATE_CONTINUE
    if (e_start_group(&state.dev) != E_OK) {
        fprintf(stderr, "ERROR: e_start_group() failed.\n");
        return 0;
    }

    // Every iteration we only have to read the start of the buffer
    // because that is where the syncstate flags are located
    // We have to read everything up to remotetimer (not included)
    const int read_size = offsetof(ebsp_combuf, remotetimer);

#ifdef DEBUG
    int cores_initialized;
    while (1) {
        _microsleep(1000); // 1 millisecond

        // Read the communication buffer
        if (e_read(&state.emem, 0, 0, 0, &state.combuf, read_size) !=
            read_size) {
            fprintf(stderr, "ERROR: e_read ebsp_combuf failed in ebsp_spmd.\n");
            return 0;
        }

        // Check every core
        cores_initialized = 0;
        for (int i = 0; i < state.nprocs; ++i)
            if (state.combuf.syncstate[i] == STATE_EREADY)
                ++cores_initialized;
        if (cores_initialized == state.nprocs)
            break;
    }
    printf("(BSP) DEBUG: All epiphany cores are ready for initialization.\n");
    printf("(BSP) DEBUG: ebsp uses %d KB = %p B of external memory.\n",
           sizeof(ebsp_combuf) / 1024, (void*)sizeof(ebsp_combuf));

    _update_remote_timer();

    // Send start signal
    for (int i = 0; i < state.nprocs; ++i)
        _write_core_syncstate(i, STATE_CONTINUE);
#endif

    int total_syncs = 0;
    int extmem_corrupted = 0;
    int run_counter;
    int sync_counter;
    int finish_counter;
    int continue_counter;
    int abort_counter;

#ifdef DEBUG
    int iter = 0;
    printf("(BSP) DEBUG: All epiphany cores initialized.\n");
#endif

    for (;;) {
        _update_remote_timer();
        _microsleep(1); // 1000 is 1 millisecond

        // Read the first part of the communication buffer
        // that contains sync states: read all up till coredata (not inclusive)
        if (e_read(&state.emem, 0, 0, 0, &state.combuf, read_size) !=
            read_size) {
            fprintf(stderr, "ERROR: e_read ebsp_combuf failed in ebsp_spmd.\n");
            return 0;
        }

        // Check interrupts
        for (int i = 0; i < state.nprocs; i++) {
            if (state.combuf.interrupts[i] != 0) {
                uint32_t ipend = state.combuf.interrupts[i];
                fprintf(stderr, "WARNING: Interrupt occured on core %d: 0x%x\n",
                        i, ipend);
                // Reset
                state.combuf.interrupts[i] = 0;
                _write_extmem((void*)&state.combuf.interrupts[i],
                              offsetof(ebsp_combuf, interrupts[i]),
                              sizeof(uint16_t));
            }
        }

        // Check sync states
        run_counter = 0;
        sync_counter = 0;
        finish_counter = 0;
        continue_counter = 0;
        abort_counter = 0;
        for (int i = 0; i < state.nprocs; i++) {
            switch (state.combuf.syncstate[i]) {
            case STATE_INIT:
                break;

            case STATE_RUN:
                run_counter++;
                break;

            case STATE_SYNC:
                sync_counter++;
                break;

            case STATE_FINISH:
                finish_counter++;
                break;

            case STATE_CONTINUE:
                continue_counter++;
                break;

            case STATE_ABORT:
                abort_counter++;
                break;

            case STATE_MESSAGE:
                printf("$%02d: %s\n", i, state.combuf.msgbuf);
                fflush(stdout);
                // Reset flag to let epiphany core continue
                _write_core_syncstate(i, STATE_CONTINUE);
                break;

            default:
                extmem_corrupted++;
                if (extmem_corrupted <= 32) // to avoid overflow
                    fprintf(stderr, "ERROR: External memory corrupted."
                                    " syncstate[%d] = %d.\n",
                            i, state.combuf.syncstate[i]);
                break;
            }
        }

#ifdef DEBUG
        if (iter % 1000 == 0) {
            printf("run %02d - sync %02d - finish %02d - continue %02d -- "
                   "iteration %d\n",
                   run_counter, sync_counter, finish_counter, continue_counter,
                   iter);
            fflush(stdout);
        }
        ++iter;
#endif

        if (sync_counter == state.nprocs_used) {
            ++total_syncs;
#ifdef DEBUG
            // This part of the sync (host side)
            // usually does not crash so only one
            // line of debug output is needed here
            printf("(BSP) DEBUG: Sync %d\n", total_syncs);
#endif
            // if call back, call and wait
            if (state.sync_callback)
                state.sync_callback();

            // First reset the combuf
            for (int i = 0; i < state.nprocs_used; i++)
                state.combuf.syncstate[i] = STATE_CONTINUE;
            _write_extmem(&state.combuf.syncstate,
                          offsetof(ebsp_combuf, syncstate),
                          NPROCS * sizeof(int));
            // Now write it to all cores to continue their execution
            for (int i = 0; i < state.nprocs_used; i++)
                _write_core_syncstate(i, STATE_CONTINUE);
        }
        if (abort_counter != 0) {
            printf("(BSP) ERROR: bsp_abort was called\n");
            break;
        }
        if (finish_counter == state.nprocs_used)
            break;
    }
    // Read the communication buffer
    // to get final messages from the program
    if (e_read(&state.emem, 0, 0, 0, &state.combuf, sizeof(ebsp_combuf)) !=
        sizeof(ebsp_combuf)) {
        fprintf(stderr,
                "ERROR: e_read full ebsp_combuf failed in ebsp_spmd.\n");
        return 0;
    }

#ifdef DEBUG
    printf("(BSP) INFO: Program finished\n");
#endif

    if (state.end_callback)
        state.end_callback();

    return 1;
}

int bsp_end() {
    if (!bsp_initialized) {
        fprintf(stderr,
                "ERROR: bsp_end called when bsp was not initialized.\n");
        return 0;
    }

    e_free(&state.emem);

    if (E_OK != e_finalize()) {
        fprintf(stderr, "ERROR: Could not finalize the Epiphany connection.\n");
        return 0;
    }

    memset(&state, 0, sizeof(state));
    bsp_initialized = 0;

    if (state.end_callback)
        state.end_callback();

    return 1;
}

int bsp_nprocs() { return state.nprocs; }

