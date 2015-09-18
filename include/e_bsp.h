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
 * and used ebsp_set_tagsize.
 * Note that the order of receiving messages is not guaranteed to correspond
 * to the order in which they were sent.
 *
 * Before the first sync, the queue contains messages from the ARM host
 * They are invalidated after the first call to bsp_sync().
 */

#pragma once

#include <stddef.h>

/**
 * Denotes the start of a BSP program.
 * This initializes the BSP system on the core,
 *
 * Must be called before calling any other BSP function.
 */
void bsp_begin();

/**
 * Denotes the end of a BSP program.
 *
 * Finalizes and cleans up the BSP program.
 * No other BSP functions are allowed to be called after this function is
 * called.
 */
void bsp_end();

/**
 * Obtain the number of processors currently in use.
 * @return An integer indicating the number of running processors
 */
int bsp_nprocs();

/**
 * Obtain the processor ID of the local core.
 * @return An integer with the id of the process
 * The pid is an integer between 0 and `bsp_nprocs()-1` inclusive.
 */
int bsp_pid();

/**
 * Obtain the (accurate) time in seconds since bsp_begin() was called.
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
 * Obtain the number of clockcycles that have passed since the previous call
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
 * Obtain the (inaccurate) time in seconds since bsp_begin() was called.
 * @return A floating point value with the amount of seconds since bsp_begin()
 *
 * This timer has much less accuracy (milliseconds at best) than bsp_time()
 * but works if time differences are more than 5 seconds in which case
 * the accurate timer does not work.
 */
float ebsp_host_time();

/**
 * Denotes the end of a superstep, and performs all communication
 * and registration.
 *
 * Serves as a blocking barrier which halts execution untill all Epiphany
 * cores are finished with the current superstep.
 */
void bsp_sync();

/**
 * Synchronizes cores without resolving outstanding communication
 */
void ebsp_barrier();

/**
 * Synchronizes with the host processor without resolving outstanding
 * communication.
 */
void ebsp_host_sync();

/**
 * Register a variable as available for remote access.
 * @param variable A pointer to the variable (local)
 * @param nbytes The size in bytes of the variable
 *
 * The operation takes effect after the next call to bsp_sync().
 * Only one registration is allowed in a single superstep.
 * When a variable is registered, every core must do so.
 *
 * The system maintains a stack of registered variables. Any variables
 * registered in the same superstep are identified with each other.
 *
 * Registering a variable needs to be done before it can be used with
 * the functions bsp_put(), bsp_hpput(), bsp_get(), bsp_hpget().
 *
 * Usage example:
 * \code{.c}
 * int a, b, c, p;
 * int x[16];
 *
 * bsp_push_reg(&a, sizeof(int));
 * bsp_sync();
 * bsp_push_reg(&x, sizeof(x));
 * bsp_sync();
 *
 * p = bsp_pid();
 *
 * // Get the value of the `a` variable of core 0 and save it in `b`
 * bsp_get(0, &a, 0, &b, sizeof(int));
 *
 * // Save the value of `c` into the array `x` on core 0, at array location p
 * bsp_put(0, &c, &x, p*sizeof(int), sizeof(int));
 * \endcode
 *
 * @remarks In the current implementation, the parameter nbytes is ignored.
 */
void bsp_push_reg(const void* variable, const int nbytes);

/**
 * De-register a variable for remote memory access.
 * @param variable A pointer to the variable, which must have been
 *  previously registered with bsp_push_reg
 *
 * The operation takes effect after the next call to bsp_sync().
 * @remarks In the current implementation, this function does nothing.
 */
void bsp_pop_reg(const void* variable);

/**
 * Copy data to another processor (buffered).
 * @param pid The pid of the target processor (can be self)
 * @param src A pointer to the source data
 * @param dst A variable that was previously registered with bsp_push_reg()
 * @param offset An offset to be added to dst
 * @param nbytes The amount of bytes to be copied
 *
 * The data in src is copied to a buffer (currently in slow external memory)
 * at the moment bsp_put is called. Therefore the caller can immediately
 * replace the data in src right after bsp_put returns.
 * When bsp_sync is called, the data will be transferred from the buffer
 * to the destination at the other processor.
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 * @remarks The current implementation uses external memory which restraints
 *          its performance greatly. Where possible use bsp_hpput() instead.
 */
void bsp_put(int pid, const void *src, void *dst, int offset, int nbytes);

/**
 * Copy data to another processor, unbuffered.
 * @param pid The pid of the target processor (can be self)
 * @param src A pointer to local source data
 * @param dst A variable that was previously registered with bsp_push_reg()
 * @param offset An offset to be added to dst
 * @param nbytes The amount of bytes to be copied
 *
 * The data is immediately copied into the destination at the remote processor,
 * as opposed to bsp_put which first copies the data to a buffer.
 * This means the programmer must make sure that the other processor is not
 * using the destination at this moment.
 * The data transfer is guaranteed to be complete after the next call to
 * bsp_sync().
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
*/
void bsp_hpput(int pid, const void *src, void *dst, int offset, int nbytes);

/**
 * Copy data from another processor using a buffer.
 * @param pid The pid of the target processor (allowed to be equal to the
 *            sending core)
 * @param src A variable that has been previously registered with bsp_push_reg
 * @param offset An offset in bytes with respect to the remote location of src
 * @param dst A pointer to a local destination
 * @param nbytes The number of bytes to be copied
 *
 * No data transaction takes place until the next call to bsp_sync, at which
 * point the data will be copied from source to destination.
 *
 * @remarks The official BSP standard dictates that first all the data of all
 * bsp_get() transactions is copied into a buffer, after which all the data is
 * written to the proper destinations. This would allow one to use bsp_get to
 * swap to variables in place. Because of memory constraints we take a
 * different approach in our implementation. The bsp_get() transactions are all
 * executed at the same time, so such a swap would result in undefined
 * behaviour.
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 */
void bsp_get(int pid, const void *src, int offset, void *dst, int nbytes);

/**
 * Copy data from another processor. It is an unbuffered version of bsp_get().
 * @param pid The pid of the target processor (can be self)
 * @param src A variable that has been previously registered with bsp_push_reg
 * @param offset An offset to be added to src
 * @param dst A pointer to a local destination
 * @param nbytes The amount of bytes to be copied
 *
 * As opposed to bsp_get(), the data is transferred immediately When
 * bsp_hpget() is called. The programmer must make sure that the source data
 * is available upon calling. Communication through this mechanism is strongly
 * preferred over buffered communication, since it is much faster than
 * bsp_get().
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 */
void bsp_hpget(int pid, const void *src, int offset, void *dst, int nbytes);

/**
 * Obtain the tag size.
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
 * Obtain the amount of messages in the queue and their total data size.
 * @param packets a pointer to an integer receiving the amount of messages
 * @param accum_bytes a pointer to an integer receiving the amount of bytes
 *
 * Upon return, the integers pointed to by packets and accum_bytes will
 * hold the amount of messages in the queue, and the sum of the sizes
 * of their data payloads respectively.
 */
void bsp_qsize(int *packets, int *accum_bytes);

/**
 * Obtain the tag and size of the next message without popping the message.
 * @param status A pointer to an integer receiving the message data size.
 * @param tag A pointer to a buffer receiving the message tag
 *
 * Upon return, the integer pointed to by status will receive the size of the
 * data payload of the next message in the queue. If there is no next message
 * it will be set to -1.
 * The buffer pointed to by tag should be large enough to store the tag.
 * The minimum size can be obtained by calling ebsp_get_tagsize.
 */
void bsp_get_tag(int *status, void *tag);

/**
 * Obtain the next message from the message queue and pop the message.
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
 * Obtain the next message, with tag, from the queue and pop the message.
 * @param tag_ptr_buf A pointer to a pointer receiving the location of the tag
 * @param payload_ptr_buf A pointer to a pointer receiving the location of the
 * data pyaload
 * @return The number of bytes of the payload data
 *
 * This function will give the user direct pointers to the tag and data
 * of the message. This avoids the data copy as done in bsp_move().
 *
 * @remarks that both tag and payload can be stored in external memory.
 * Repeated use of these tags will lead to overall worse performance, such that
 * bsp_move() can actually outperform this variant.
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
 * @remarks 
 * - ebsp_send_up() should an only be used between the last call to bsp_sync()
 *   and bsp_end()
 * - ebsp_send_up() should only be used when no bsp_send() messages have been
 *   passed after the last bsp_sync()
 * - after calling ebsp_send_up() at least once, a call to any other
 *   queue functions or to bsp_sync() will lead to undefined behaviour
 */
void ebsp_send_up(const void *tag, const void *payload, int nbytes);

/**
 * Wait for any input-DMAs to finish.
 * A pointer to a chunk of input data is written to *address,
 * the size of this chunk is returned. stream_id is the index of the 
 * input stream sent to this core, in the same order as ebsp_send_buffered().
 * prealloc can be set to either 1 (true) or 0 (false), and determines whether double
 * or single buffering is used.
 *
 * @remarks
 * - Sets *address=0 and returns 0 if the stream has ended
 * - Uses the DMA engine
 */
int ebsp_move_chunk_down(void** address, unsigned stream_id, int prealloc);

/**
 * Wait for any output-DMAs to finish.
 * A pointer to a chunk of empty memory is written to *address,
 * the size of this chunk is returned. stream_id is the index of the 
 * output stream, in the same order as ebsp_send_buffered().
 * prealloc can be set to either 1 (true) or 0 (false), and determines whether double
 * or single buffering is used.
 *
 * @remarks
 * - Uses the DMA engine
 */
int ebsp_move_chunk_up(void** address, unsigned stream_id, int prealloc);


// TODO: write abstract
void ebsp_move_down_cursor(int stream_id, int jump_n_chunks);

// TODO: write abstract
void ebsp_reset_down_cursor(int stream_id);


int ebsp_open_up_stream(void** address, unsigned stream_id);
void ebsp_close_up_stream(unsigned stream_id);
int ebsp_open_down_stream(void** address, unsigned stream_id);
void ebsp_close_down_stream(unsigned stream_id);

/**
 * Sets the number of bytes that has to be written from the current output chunk to extmem.
 * The default value is max_chunk_size
 */
void ebsp_set_up_chunk_size(unsigned stream_id, int nbytes);

/**
 * Aborts the program after outputting a message.
 * @param format The formatting string in printf style
 *
 * bsp_abort aborts the program after outputting a message.
 * This terminates all running epiphany-cores regardless of their status.
 */

// The attributes in this definition make sure that the compiler checks the
// arguments for errors.
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
 * Allocate local memory.
 * @param nbytes The size of the memory block
 *
 * This function allocates memory in local SRAM, meaning the memory is fast
 * but extremely limited.
 */
void* ebsp_malloc(unsigned int nbytes);

/**
 * Free allocated external or local memory.
 * @param ptr A pointer to memory previously allocated by ebsp_ext_malloc()
 *            or by ebsp_malloc()
 */
void ebsp_free(void* ptr);


#define ALIGN(x)    __attribute__ ((aligned (x)))

typedef struct
{
    unsigned config;
    unsigned inner_stride;
    unsigned count;
    unsigned outer_stride;
    void    *src_addr;
    void    *dst_addr;
} ALIGN(8) ebsp_dma_handle;

/**
 * Push a new task to the DMA engine
 * @param desc   Used in combination with ebsp_dma_wait(). It is completely filled by this function
 * @param dst    Destination address
 * @param src    Source address
 * @param nbytes Amount of bytes to be copied
 *
 * Assumes previous task in `desc` is completed (use ebsp_dma_wait())
 */
void ebsp_dma_push(ebsp_dma_handle* desc, void *dst, const void *src, size_t nbytes);

/**
 * Wait for the task to be completed.
 */
void ebsp_dma_wait(ebsp_dma_handle* desc);

/**
 * Get a raw remote memory address for a variable
 * that was registered using bsp_push_reg()
 * @param pid Remote core id
 * @param variable An address that was registered using bsp_push_reg
 * @return A pointer to the remote variable, or 0 if it was not registered
 */
void* ebsp_get_raw_address(int pid, const void* variable);

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

