/*
file: e_bsp.h

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

/**
 * @file e_bsp.h
 * @brief All BSP core functions for the Epiphany cores.
 *
 * This file contains all BSP core functions.
 *
 * ## BSP Message Passing functions
 * Every message contains a fixed-length (can change per superstep)
 * tag and a variable-length payload.
 * Default tag-size is zero unless the host has sent messages
 * and used ebsp_set_tagsize *
 * The order of receiving messages is not guaranteed
 *
 * Before the first sync, the queue contains messages from the ARM host
 * They are invalidated after the first call to bsp_sync
 */


#pragma once

/**
 * Start the BSP program.
 *
 * Must be called before calling any other BSP function.
 */
void bsp_begin();

/**
 * Finalize and clean up the BSP program.
 *
 * No other BSP functions can be called after this function is called.
 */
void bsp_end();

/**
 * Get the number of available processors.
 * @return An integer indicating the number of available processors
 */
int bsp_nprocs();

/**
 * Get the processor ID of the local core.
 * @return An integer with the id of the process
 */
int bsp_pid();

/**
 * Get the (accurate) time in seconds since bsp_begin() was called.
 * @return A floating point value with the amount of seconds since bsp_begin()
 *
 * The epiphany-timer does not support time differences longer than
 * `UINT_MAX/(600000000)` which is roughly 5 seconds.
 *
 * For longer time differences, use the less accurate ebsp_host_time().
 *
 * Do not use this in combination with ebsp_raw_time(), use only one of them.
 */
float bsp_time();

/**
 * Get the number of clockcycles that have passed since the previous call
 * to ebsp_raw_time().
 * @return An unsigned integer with the amount of clockcycles
 *
 * This function has less overhead than bsp_time.
 *
 * Divide the amount of clockcycles by 600 000 000 to get the time in seconds.
 *
 * Do not use this in combination with bsp_time(), use only one of them.
 */
unsigned int ebsp_raw_time();

/**
 * Get the (inaccurate) time in seconds since bsp_begin() was called.
 * @return A floating point value with the amount of seconds since bsp_begin()
 *
 * This timer has much less accuracy (milliseconds at best) than bsp_time()
 * but works if time differences are more than 5 seconds in which case
 * the accurate timer does not work.
 */
float ebsp_host_time();

/**
 * Terminates a superstep, and starts all communication.
 * The computation is halted until all communication has been performed.
 */
void bsp_sync();

/**
 * Register a variable as available for remote access.
 * @param variable A pointer to the variable
 * @param nbytes The size in bytes of the variable
 *
 * The operation takes effect after the next call to bsp_sync
 * Every core must register the variables, one per syncrhonization step,
 * in the same order.
 * Registering a variable needs to be done before it can be used with
 * the functions bsp_put(), bsp_hpput(), bsp_get(), bsp_hpget().
 *
 * In the current implementation, the parameter nbytes is ignored.
 */
void bsp_push_reg(const void* variable, const int nbytes);

/**
 * De-register a variable for remote memory access.
 * @param variable A pointer to the variable, which must have been
 *  previously registered with bsp_push_reg
 *
 * The operation takes effect after the next call to bsp_sync
 * In the current implementation, this function does nothing.
 */
void bsp_pop_reg(const void* variable);

/**
 * Copy data to another processor (buffered).
 * @param pid The pid of the target processor (can be self)
 * @param src A pointer to the source data
 * @param dst A variable that has been previously registered with bsp_push_reg
 * @param offset An offset to be added to dst
 * @param nbytes The amount of bytes to be copied
 *
 * The data in src is copied to a buffer (currently in slow external memory)
 * at the moment bsp_put is called. Therefore the caller can immediately
 * replace the data in src right after bsp_put returns.
 * When bsp_sync is called, the data will be transferred from the buffer
 * to the destination at the other processor.
 *
 * The current implementation does not check if nbytes is valid.
 * The current implementation is slow because it uses external memory,
 * so use bsp_hpput if possible.
 */
void bsp_put(int pid, const void *src, void *dst, int offset, int nbytes);

/**
 * Copy data to another processor (unbuffered).
 * @param pid The pid of the target processor (can be self)
 * @param src A pointer to local source data
 * @param dst A variable that has been previously registered with bsp_push_reg
 * @param offset An offset to be added to dst
 * @param nbytes The amount of bytes to be copied
 *
 * The data is immediately copied into the destination at the remote processor,
 * as opposed to bsp_put which first copies the data to a buffer.
 * This means the programmer must make sure that the other processor is not
 * using the destination at this moment.
 * The data transfer is guaranteed to be complete after the next call to
 * bsp_sync.
 *
 * The current implementation does not check if nbytes is valid.
*/
void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes);

/**
 * Copy data from another processor (buffered).
 * @param pid The pid of the target processor (can be self)
 * @param src A variable that has been previously registered with bsp_push_reg
 * @param offset An offset to be added to src
 * @param dst A pointer to a local destination
 * @param nbytes The amount of bytes to be copied
 *
 * No data transaction takes place untill the next call to bsp_sync, at which
 * the data will be copied from source to destination.
 *
 * The official BSP standard dictates that first all the data of all bsp_get
 * transactions is copied into a buffer, after which all the data is written
 * to the proper destinations. This would allow one to use bsp_get to swap
 * to variables in place.
 * This implementation does NOT adhere to this standard. The bsp_get
 * transactions are all executed at the same time, so such a swap would
 * result in undefined behaviour.
 *
 * The current implementaiton does not check if nbytes is valid.
 */
void bsp_get(int pid, const void *src, int offset, void *dst, int nbytes);

/**
 * Copy data from another processor (unbuffered).
 * @param pid The pid of the target processor (can be self)
 * @param src A variable that has been previously registered with bsp_push_reg
 * @param offset An offset to be added to src
 * @param dst A pointer to a local destination
 * @param nbytes The amount of bytes to be copied
 *
 * As opposed to bsp_get, the data is transferred immediately when bsp_hpget
 * is called. The programmer must make sure that the source data is available.
 * This function is faster than bsp_get.
 *
 * The current implementaiton does not check if nbytes is valid.
 */
void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes);

/**
 * Get the tag size.
 * @return The tag size in bytes
 *
 * This function gets the tag size, valid for this superstep.
 */
int ebsp_get_tagsize();

/**
 * Set the tag size.
 * @param tag_bytes A pointer to the tag size, in bytes
 *
 * Upon return, the value pointed to by tag_bytes will contain
 * the old tag size. The new tag size will take effect at the next superstep,
 * so messages that are being sent in this superstep will have the old tag size.
 */
void bsp_set_tagsize(int *tag_bytes);

/**
 * Send a message to another processor.
 * @param pid The pid of the target processor (can be self)
 * @param tag A pointer to the tag data
 * @param payload A pointer to the data
 * @param nbytes The size of the data
 *
 * This will send a message to the target processor, using the message passing
 * system. The tag size can be obtained by ebsp_get_tagsize.
 * When this function returns, the data has been copied so the user can
 * use the buffer for other purposes.
 */
void bsp_send(int pid, const void *tag, const void *payload, int nbytes);

/**
 * Get the amount of messages in the queue and their total data size.
 * @param packets a pointer to an integer receiving the amount of messages
 * @param accum_bytes a pointer to an integer receiving the amount of bytes
 *
 * Upon return, the integers pointed to by packets and accum_bytes will
 * hold the amount of messages in the queue, and the sum of the sizes
 * of their data payloads respectively.
 */
void bsp_qsize(int *packets, int *accum_bytes);

/**
 * Get the tag and size of the next message without popping the message.
 * @param status A pointer to an integer receiving the message data size.
 * @param tag A pointer to a buffer receiving the message tag
 *
 * Upon return, the integer pointed to by status will receive the size of the
 * data payload of the next message in the queue. If there is no next message
 * it will be set to -1.
 * The buffer pointed to by tag should be big enough. The minimum size is
 * obtained by calling ebsp_get_tagsize.
 */
void bsp_get_tag(int *status, void *tag);

/**
 * Get the next message from the message queue and pop the message.
 * @param payload A pointer to a buffer receiving the data payload
 * @param buffer_size The size of the buffer
 *
 * This will copy the payload and pop the message from the queue.
 * The size of the payload can be obtained by calling bsp_get_tag().
 * If `buffer_size` is smaller than the data payload then the data is
 * truncated.
 */
void bsp_move(void *payload, int buffer_size);

/**
 * Get the next message, with tag, from the queue and pop the message.
 * @param tag_ptr_buf A pointer to a pointer receiving the location of the tag
 * @param payload_ptr_buf A pointer to a pointer receiving the location of the
 * data pyaload
 * @return The number of bytes of the payload data
 *
 * This function will give the user direct pointers to the tag and data
 * of the message. This avoids the data copy as done in bsp_move().
 *
 * Note that both tag and payload can be stored in slow external memory.
 */
int bsp_hpmove(void **tag_ptr_buf, void **payload_ptr_buf);

/**
 * Send a message to the host processor after the computation is finished.
 * @param tag A pointer to the tag data
 * @param payload A pointer to the data
 * @param nbytes The size of the data
 *
 * This will send a message back to the host after a computation. It is used
 * to tranfer any results.
 *
 * When this function returns, the data has been copied so the user can
 * use the buffer for other purposes.
 *
 * ## Usage restrictions:
 * - ebsp_send_up() can only be used between the last call bsp_sync() and
 *   bsp_end()
 * - ebsp_send_up() can only be used when no bsp_send() messages have been
 *   passed after the last bsp_sync()
 * - after calling ebsp_send_up() at least once, a call to any other
 *   queue functions or to bsp_sync() will lead to undefined results
 */
void ebsp_send_up(const void *tag, const void *payload, int nbytes);

/**
 * Aborts the program after outputting a message.
 * @param format The formatting string in printf style
 *
 * bsp_abort aborts the program after outputting a message
 * This terminates all running epiphany-cores regardless of their status
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors
 */
void bsp_abort(const char * format, ...)
        __attribute__((__format__(__printf__, 1, 2)));

/**
 * Allocate external memory.
 * @param nbytes The size of the memory block
 *
 * This function allocates memory in external RAM, meaning the memory is slow
 * and should not be used with time critical computations.
 */
void* ebsp_ext_malloc(unsigned int nbytes);

/**
 * Free allocated external memory.
 * @param ptr A pointer to memory previously allocated by ebsp_ext_malloc()
 */
void ebsp_free(void* ptr);

/**
 * Output a debug message printf style.
 * @param format The formatting string in printf style
 *
 * ebsp_message outputs a debug message by sending it to shared memory
 * So that the host processor can output it to the terminal
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors.
 */
void ebsp_message(const char * format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

