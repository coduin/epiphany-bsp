/*
File: e_bsp.h

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

#include "common.h"
#include "_ansi.h"

/** Starts the BSP program.
*/
void bsp_begin();

/** Finalizes and cleans up the BSP program.
*/
void bsp_end();

/** Returns the number of available processors.
 *
 *  @return nprocs: An integer indicating the number of available processors.
 */
int bsp_nprocs();

/** Returns the processor ID of the local core
 *
 *  pid: An integer indicating the id of the local core
 */
int bsp_pid();

/** Time in seconds that has passed since bsp_begin() was called
 * The epiphany-timer does not support time differences longer than
 * UINT_MAX/(600000000) which is roughly 5 seconds.
 * For longer time differences, use the less accurate bsp_remote_time.
 * Do not use this in combination with bsp_raw_time, use only one of them.
 */
float bsp_time();

/* Returns the number of clockcycles that have passed since the previous call
 * to bsp_raw_time. This has less overhead than bsp_time.
 * Divide the amount of clockcycles by 600 000 000 to get the time in seconds.
 * Do not use this in combination with bsp_time, use only one of them.
 */
unsigned int bsp_raw_time();

/* Time in seconds since bsp_begin() was called.
 * This time has much less accuracy (milliseconds at best)
 * But works if time differences are more than 5 seconds
 */
float bsp_remote_time();

/** Terminates a superstep, and starts all communication. The computation is 
 *  halted until all communication has been performed.
 */
void bsp_sync();

/** Registers a variable. Takes effect at the next sync.
*/
void bsp_push_reg(const void* variable, const int nbytes);

/* De-registers a variable. Takes effect at the next sync.
*/
void bsp_pop_reg(const void* variable);

/** Put a variable to another processor
 * Buffered version: the data in src is saved at the moment of the call
 * and it is transferred to the other core at the next sync
 */
void bsp_put(int pid, const void *src, void *dst, int offset, int nbytes);

/** Unbuffered version of bsp_put. The data is transferred immediately.
*/
void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes);

/** Gets a variable from a processor.
 * Buffered version: the communication occurs during the next sync
 * So after calling bsp_get the caller does not get the data untill
 * the next bsp_sync has finished.
 */
void bsp_get(int pid, const void *src, int offset, void *dst, int nbytes);

/** Unbuffered version of bsp_get
 * The data is tranferred immediately and there is no guarantee
 * about the state of the other processor at that moment.
 */
void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes);


/* BSP Message Passing functions
 * Every message contains a fixed-length (can change per superstep)
 * tag and a variable-length payload.
 * Default tag-size is zero unless the host has sent messages
 * and used ebsp_set_tagsize
 *
 * The order of receiving messages is not guaranteed
 *
 * Before the first sync, the queue contains messages from the ARM host
 * They are invalidated after the first call to bsp_sync
 */
int ebsp_get_tagsize();
void bsp_set_tagsize(int *tag_bytes);
void bsp_send(int pid, const void *tag, const void *payload, int nbytes);
void bsp_qsize(int *packets, int *accum_bytes);
void bsp_get_tag(int *status, void *tag);
void bsp_move(void *payload, int buffer_size);
int bsp_hpmove(void **tag_ptr_buf, void **payload_ptr_buf);

/* ebsp_send_up is used to send messages back to the host after the computation
 * is finished. It is used to transfer the results of a computation
 *
 * Usage restrictions:
 * - ebsp_send_up can only be used between the last bsp_sync and bsp_end
 * - ebsp_send_up can only be used when no bsp_send messages have been passed
 *   after the last sync
 * - after calling ebsp_send_up at least once, a call to any other
 *   queue functions or to bsp_sync will lead to undefined results
 */
void ebsp_send_up(const void *tag, const void *payload, int nbytes);

/** bsp_abort aborts the program after outputting a message
 * This terminates all running epiphany-cores regardless of their status
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors
 */
void _EXFUN(bsp_abort, (const char *, ...)
        _ATTRIBUTE((__format__(__printf__, 1, 2))));

/*
 * Allocate external memory.
 * Keep in mind that this memory is slow so should not be used
 * for floating point computations
 */
void* ebsp_ext_malloc(unsigned int nbytes);

/*
 * Free allocated external memory.
 */
void ebsp_free(void* ptr);

/** ebsp_message outputs a debug message by sending it to shared memory
 * So that the host processor can output it to the terminal
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors
 */
void _EXFUN(ebsp_message, (const char *, ...)
    _ATTRIBUTE((__format__(__printf__, 1, 2))));

