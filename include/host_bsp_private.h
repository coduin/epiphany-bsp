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

#pragma once

#include "host_bsp.h"
#include "ebsp_common.h"

#define __USE_XOPEN2K
#define __USE_POSIX199309 1
#include <time.h>

#define MAX_N_STREAMS 1000

#ifdef DEBUG
typedef struct {
    int index;
    unsigned int value; // must be unsigned, addresses use the last bit
    size_t size;
    int type;    // STT_FUNC, STT_OBJECT, etc
    int bind;    // STB_GLOBAL, STB_LOCAL
    int section; // SHN_ABS, SHN_COMMON, SHN_UNDEF, or section index
    char name[64];
} Symbol;
#endif

/*
 *  Global BSP state
 */

typedef struct {
    // The number of processors available
    int nprocs;

    // The directory of the program and e-program
    // including a trailing slash
    char e_directory[1024];
    // The name of the e-program
    char e_fullpath[1024];

    // Number of rows or columns in use
    int rows;
    int cols;

    // Number of processors in use
    int nprocs_used;

    // Epiphany structure to describe external memory mmap info
    e_mem_t emem;

    // memory-mapped pointers to external memory
    // They are the host-side version of E_XXX_ADDR in common.h
    void* host_combuf_addr;
    void* host_dynmem_addr;

    // Local copy of ebsp_combuf to copy from and copy into.
    ebsp_combuf combuf;
    // For reading out the final queue after spmd
    int message_index;

    void (*sync_callback)(void);
    void (*end_callback)(void);

    int num_vars_registered;

    // Epiphany specific variables
    e_platform_t platform;
    e_epiphany_t dev;

    // Timer storage
    struct timespec ts_start, ts_end;

    // Buffer
    ebsp_stream_descriptor buffered_streams[NPROCS][MAX_N_STREAMS];

#ifdef DEBUG
    Symbol* e_symbols;
    int num_symbols;
#endif

} bsp_state_t;

extern bsp_state_t state;

/*
 *  host_bsp
 */
int bsp_init(const char* _e_name, int argc, char** argv);
int bsp_begin(int nprocs);
int ebsp_spmd();
int bsp_end();
int bsp_nprocs();

/*
 *  host_bsp_memory
 */
void ebsp_malloc_init();
void* ebsp_ext_malloc(unsigned int nbytes);
void ebsp_free(void* ptr);
int ebsp_write(int pid, void* src, off_t dst, int size);
int ebsp_read(int pid, off_t src, void* dst, int size);
int _write_core_syncstate(int pid, int syncstate);
int _write_extmem(void* src, off_t offset, int size);

/*
 *  host_bsp_buffer
 */
void ebsp_send_buffered(void* src, int dst_core_id, int nbytes,
                        int max_chunksize);
void ebsp_send_buffered_raw(void* src, int dst_core_id, int nbytes,
                            int max_chunksize);
void* ebsp_get_buffered(int src_core_id, int nbytes, int max_chunksize);
void _ebsp_add_stream(int dst_core_id, void* extmem_in_buffer, int nbytes,
                      int max_chunksize, int is_instream);
void ebsp_create_down_stream_raw(const void* src, int dst_core_id, int nbytes,
                                 int max_chunksize);

/*
 *  host_bsp_mp
 */
void ebsp_set_tagsize(int* tag_bytes);
void ebsp_send_down(int pid, const void* tag, const void* payload, int nbytes);
int ebsp_get_tagsize();
void ebsp_qsize(int* packets, int* accum_bytes);
ebsp_message_header* _next_queue_message();
void _pop_queue_message();
void ebsp_get_tag(int* status, void* tag);
void ebsp_move(void* payload, int buffer_size);
int ebsp_hpmove(void** tag_ptr_buf, void** payload_ptr_buf);

/*
 *  host_bsp_utility
 */
void ebsp_set_sync_callback(void (*cb)());
void ebsp_set_end_callback(void (*cb)());
void* _arm_to_e_pointer(void* ptr);
void* _e_to_arm_pointer(void* ptr);
void _update_remote_timer();
void _microsleep(int microseconds);
void _get_p_coords(int pid, int* row, int* col);
void init_application_path();

/*
 * host_bsp_debug
 */
#ifdef DEBUG
void _read_elf(const char* filename);
Symbol* _get_symbol_by_addr(void* addr);
Symbol* _get_symbol_by_name(const char* symbol);
#endif
