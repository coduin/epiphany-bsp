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
#include "common.h"

/**
 * Denotes the start of a BSP program.
 * This initializes the BSP system on the core.
 *
 * Must be called before calling any other BSP function.
 * Should only be called once in a program.
 */
void bsp_begin();

/**
 * Denotes the end of a BSP program.
 *
 * Finalizes and cleans up the BSP program.
 * No other BSP functions are allowed to be called after this function is
 * called.
 *
 * @remarks Must be followed by a return statement in your main function if you
 * want to call `ebsp_spmd()` multiple times.
 */
void bsp_end();

/**
 * Obtain the number of Epiphany cores currently in use.
 * @return An integer indicating the number of cores on which the program runs.
 */
int bsp_nprocs();

/**
 * Obtain the processor identifier of the local core.
 * @return An integer with the id of the core
 * The processor id is an integer in the range [0, .., bsp_nprocs() - 1].
 */
int bsp_pid();

/**
 * Obtain the time in seconds since bsp_begin() was called.
 * @return A floating point value with the number of elapsed seconds since
 * the call to bsp_begin()
 *
 * The native Epiphany timer does not support time differences longer than
 * `UINT_MAX/(600000000)` which is roughly 5 seconds.
 *
 * If you want to measure longer time intervals, we suggest you use the
 * (less accurate) ebsp_host_time().
 *
 * @remarks Using this in combination with ebsp_raw_time() leads to unspecified
 * behaviour, you should only use one of these in your program.
 */
float bsp_time();

/**
 * Obtain the number of clockcycles that have passed since the previous call
 * to ebsp_raw_time().
 * @return An unsigned integer with the number of clockcycles
 *
 * This function has less overhead than bsp_time.
 *
 * Divide the number of clockcycles by 600 000 000 to get the time in seconds.
 *
 * * @remarks Using this in combination with bsp_time() leads to unspecified
 * behaviour, you should only use one of these in your program.
 */
unsigned int ebsp_raw_time();

/**
 * Obtain the time in seconds since bsp_begin() was called.
 * @return A floating point value with the number of seconds since bsp_begin()
 *
 * This timer is much less accurate than ebsp_time(), its accuracy is in the
 * order of milliseconds. However, this timer supports time intervals of
 * arbitrary lengths.
 */
float ebsp_host_time();

/**
 * Denotes the end of a superstep, and performs all outstanding communications
 * and registrations.
 *
 * Serves as a blocking barrier which halts execution until all Epiphany
 * cores are finished with the current superstep.
 *
 * If only a synchronization is required, and you do not want the outstanding
 * communications and registrations to be resolved, then we suggest you use the
 * more efficient function ebsp_barrier()
 */
void bsp_sync();

/**
 * Synchronizes cores without resolving outstanding communication.
 *
 * This function is more efficient than bsp_sync().
 */
void ebsp_barrier();

/**
 * Synchronizes with the host processor without resolving outstanding
 * communication.
 *
 * This can be used in combination with the function ebsp_set_sync_callback()
 * for the host program to intervene in running programs on the Epiphany using
 * the host processor.
 */
void ebsp_host_sync();

/**
 * Register a variable as available for remote access.
 * @param variable A pointer to the local variable
 * @param nbytes The size in bytes of the variable
 *
 * The operation takes effect after the next call to bsp_sync().
 * Only one registration is allowed in a single superstep.
 * When a variable is registered, every core must do so.
 *
 * The system maintains a stack of registered variables. Any variables
 * registered in the same superstep are identified with each other. There
 * is a maximum number of allowed registered variables at any given time,
 * the specific number is platform dependent. This limit will be lifted
 * in a future version.
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
 * In future versions it will be used to make communication more efficient.
 */
void bsp_push_reg(const void* variable, const int nbytes);

/**
 * De-register a variable for remote memory access.
 * @param variable A pointer to the variable, which must have been
 *  previously registered with bsp_push_reg()
 *
 * The operation takes effect after the next call to bsp_sync().
 * @remarks In the current implementation, this function does
 * nothing. In a future update this function will free up variable
 * spots for new registrations.
 */
void bsp_pop_reg(const void* variable);

/**
 * Copy data to another processor (buffered).
 * @param pid The pid of the target processor (this is allowed to be the id
 *  of the sending processor)
 * @param src A pointer to the source data
 * @param dst A variable location that was previously registered using
 *  bsp_push_reg()
 * @param offset The offset in bytes to be added to the remote location
 *  corresponding to the variable location `dst`
 * @param nbytes The number of bytes to be copied
 *
 * The data in src is copied to a buffer (currently in the inefficient
 * external memory)  at the moment bsp_put is called. Therefore the caller can
 * replace the data in src right after bsp_put returns.
 * When bsp_sync() is called, the data will be transferred from the buffer
 * to the destination at the other processor.
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 * @remarks The current implementation uses external memory which restrains
 *          the performance of this function greatly. We suggest you use 
 *          bsp_hpput() wherever possible to ensure good performance.
 */
void bsp_put(int pid, const void* src, void* dst, int offset, int nbytes);

/**
 * Copy data to another processor, unbuffered.
 * @param pid The pid of the target processor (this is allowed to be the id
 *  of the sending processor)
 * @param src A pointer to local source data
 * @param dst A variable location that was previously registered using
 *  bsp_push_reg()
 * @param offset The offset in bytes to be added to the remote location
 *  corresponding to the variable location `dst`
 * @param nbytes The number of bytes to be copied
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
void bsp_hpput(int pid, const void* src, void* dst, int offset, int nbytes);

/**
 * Copy data from another processor (buffered)
 * @param pid The pid of the target processor (this is allowed to be the id
 *  of the sending processor)
 * @param src A variable that has been previously registered using
 *  bsp_push_reg()
 * @param dst A pointer to a local destination
 * @param offset The offset in bytes to be added to the remote location
 *  corresponding to the variable location `src`
 * @param nbytes The number of bytes to be copied
 *
 * No data transaction takes place until the next call to bsp_sync, at which
 * point the data will be copied from source to destination.
 *
 * @remarks The official BSP standard dictates that first all the data of all
 * bsp_get() transactions is copied into a buffer, after which all the data is
 * written to the proper destinations. This would allow one to use bsp_get to
 * swap to variables in place. Because of memory constraints we do not comply
 * with the standard. In our implementation. The bsp_get() transactions are all
 * executed at the same time, therefore such a swap would result in undefined
 * behaviour.
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 */
void bsp_get(int pid, const void* src, int offset, void* dst, int nbytes);

/**
 * Copy data from another processor. This function is the unbuffered version of
 * bsp_get().
 * @param pid The pid of the target processor (this is allowed to be the id
 *  of the sending processor)
 * @param src A variable that has been previously registered using
 *  bsp_push_reg()
 * @param dst A pointer to a local destination
 * @param offset The offset in bytes to be added to the remote location
 *  corresponding to the variable location `src`
 * @param nbytes The number of bytes to be copied
 *
 * As opposed to bsp_get(), the data is transferred immediately When
 * bsp_hpget() is called. When using this function you must make sure that the
 * source data is available and prepared upon calling. For performance reasons,
 * communication using this function should be preferred over buffered
 * communication.
 *
 * @remarks No warning is thrown when nbytes exceeds the size of the variable
 *          src.
 */
void bsp_hpget(int pid, const void* src, int offset, void* dst, int nbytes);

/**
 * Obtain the tag size.
 * @return The tag size in bytes
 *
 * This function gets the tag size currently in use. This tagsize remains valid
 * until the start of the next superstep.
 */
int ebsp_get_tagsize();

/**
 * Set the tag size.
 * @param tag_bytes A pointer to the tag size, in bytes
 *
 * Upon return, the value pointed to by tag_bytes will contain
 * the old tag size. The new tag size will take effect in the next superstep,
 * so that messages sent in this superstep will have the old tag size.
 */
void bsp_set_tagsize(int* tag_bytes);

/**
 * Send a message to another processor.
 * @param pid The pid of the target processor (this is allowed to be the id
 *  of the sending processor)
 * @param tag A pointer to the tag data
 * @param payload A pointer to the data payload
 * @param nbytes The size of the data payload
 *
 * This will send a message to the target processor, using the message passing
 * system. The tag size can be obtained by ebsp_get_tagsize.
 * When this function returns, the data has been copied so the user can
 * use the buffer for other purposes.
 */
void bsp_send(int pid, const void* tag, const void* payload, int nbytes);

/**
 * Obtain The number of messages in the queue and the combined size in bytes
 *  of their data
 * @param packets A pointer to an integer which will be overwritten with  the
 *  number of messages
 * @param accum_bytes A pointer to an integer which will be overwritten with
 *  the combined number of bytes of the message data.
 *
 * Upon return, the integers pointed to by packets and accum_bytes will
 * hold the number of messages in the queue, and the sum of the sizes
 * of their data payloads respectively.
 */
void bsp_qsize(int* packets, int* accum_bytes);

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
void bsp_get_tag(int* status, void* tag);

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
void bsp_move(void* payload, int buffer_size);

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
int bsp_hpmove(void** tag_ptr_buf, void** payload_ptr_buf);

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
void ebsp_send_up(const void* tag, const void* payload, int nbytes);

/**
 * Get the next chunk of data in a stream.
 * @param address On completion, contains a pointer to the data chunk.
 * @param stream_id Id of the input stream sent to this core, determined by
 * the order in which ebsp_create_down_stream() was called. TODO refer to
 * general page on streams for what id means.
 * @param prealloc Double buffering is used iff this parameter is nonzero.
 * @return Amount of bytes of the obtained chunk. Zero if stream has
 * finished or an error has occurred.
 *
 * TODO: more detailed description on double buffering and stream_id
 * TODO: some remarks on how DMA engine is used
 */
int ebsp_move_chunk_down(void** address, unsigned stream_id, int prealloc);

/**
 * TODO: fix this:
 * Wait for any output-DMAs to finish.
 * A pointer to a chunk of empty memory is written to *address,
 * the size of this chunk is returned. stream_id is the index of the
 * output stream, in the same order as ebsp_send_buffered().
 * prealloc can be set to either 1 (true) or 0 (false), and determines whether
 *double
 * or single buffering is used.
 *
 * @remarks
 * - Uses the DMA engine
 */
int ebsp_move_chunk_up(void** address, unsigned stream_id, int prealloc);

// TODO
void ebsp_move_down_cursor(int stream_id, int jump_n_chunks);

// TODO
void ebsp_reset_down_cursor(int stream_id);

int ebsp_open_up_stream(void** address, unsigned stream_id);
void ebsp_close_up_stream(unsigned stream_id);
int ebsp_open_down_stream(void** address, unsigned stream_id);
void ebsp_close_down_stream(unsigned stream_id);

/**
 * Sets the number of bytes that has to be written from the current output
 * chunk to external memory.
 * @param stream_id Stream id, see general page on streams TODO !!!
 * @param nbytes TODO
 *
 * The default value is `max_chunk_size`
 * TODO: what is max_chunk_size ?
 */
void ebsp_set_up_chunk_size(unsigned stream_id, int nbytes);

/**
 * Aborts the program after outputting a message.
 * @param format The formatting string in printf style
 *
 * bsp_abort aborts the program after outputting a message.
 * This terminates all running epiphany-cores regardless of their status.
 *
 * @remarks
 * After bsp_abort the cores are left in a state that does NOT allow them
 * to restart with another call to ebsp_spmd(). Instead the program has to
 * be completely reloaded to the cores, meaning bsp_end(), bsp_init()
 * and bsp_begin() have to be called again which is slow.
 *
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors.
 */
void bsp_abort(const char* format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

/**
 * Allocate external memory.
 * @param nbytes The size of the memory block
 * @return A pointer to the allocated memory, guaranteed to be 8-byte aligned
 * to ensure fast transfers, or zero on error.
 *
 * This function allocates memory in external RAM, meaning the memory is slow
 * and should not be used with time critical computations.
 *
 * When no more space is available, the function will return zero.
 * Note that it is not allowed to call ebsp_free() with a zero pointer so
 * this should always be checked.
 */
void* ebsp_ext_malloc(unsigned int nbytes);

/**
 * Allocate local memory.
 * @param nbytes The size of the memory block
 * @return A pointer to the allocated memory, guaranteed to be 8-byte aligned
 * to ensure fast transfers, or zero on error.
 *
 * This function allocates memory in local SRAM, meaning the memory is fast
 * but extremely limited.
 *
 * When no more space is available, the function will return zero.
 * Note that it is not allowed to call ebsp_free() with a zero pointer so
 * this should always be checked.
 */
void* ebsp_malloc(unsigned int nbytes);

/**
 * Free allocated external or local memory.
 * @param ptr A pointer to memory previously allocated by ebsp_ext_malloc()
 *            or by ebsp_malloc()
 *
 * Note that the malloc functions can return null pointers on error, and
 * ebsp_free will crash on null pointers.
 */
void ebsp_free(void* ptr);

/**
 * Push a new task to the DMA engine. TODO: see general page on DMA which
 * explains that it allows transfer+computation simultaneously
 * @param desc   Used in combination with ebsp_dma_wait(). Should be seen
 * as a *handle* to the task. Its contents are populated by this function.
 * @param dst    Destination address
 * @param src    Source address
 * @param nbytes Amount of bytes to be copied
 *
 * Assumes previous task in `desc` is completed (use ebsp_dma_wait())
 *
 * The DMA (1) will be started if it was not started yet.
 * If it was already started, this task will be pushed to a queue so that it
 * will be done some time later. Use ebsp_dma_wait() to wait for the task to
 * complete.
 *
 * @remarks
 * The `desc` pointer should be 8-byte aligned or behaviour is undefined.
 * This should not be a problem because the malloc functions always return
 * 8-byte aligned pointers, and having an `ebsp_dma_handle` struct as
 * local variable will be 8-byte aligned as well.
 */
void ebsp_dma_push(ebsp_dma_handle* desc, void* dst, const void* src,
                   size_t nbytes);

/**
 * Wait for the task to be completed.
 * @param desc Handle for a task. See ebsp_dma_push().
 * 
 * Use somewhere after ebsp_dma_push().
 * This function blocks untill the task in `desc` is completed.
 */
void ebsp_dma_wait(ebsp_dma_handle* desc);

/**
 * Get a raw remote memory address for a variable that was registered
 * using bsp_push_reg()
 * @param pid Remote core id
 * @param variable An address that was registered using bsp_push_reg()
 * @return A pointer to the remote variable, or 0 if it was not registered
 *
 * The returned pointer (if nonzero) can be written to and read from directly.
 * Note that the data will be transferred directly, as in bsp_hpput(),
 * so synchronization issues should be considered.
 *
 * This function is meant to be used in combination with ebsp_dma_push()
 * to transfer data between cores while doing computations at the same time.
 */
void* ebsp_get_raw_address(int pid, const void* variable);

/**
 * Performs a memory copy completely analogous to the standard C memcpy().
 * @param dst    Destination address
 * @param src    Source address
 * @param nbytes Amount of bytes to be copied
 *
 * This function is provided because the default `memcpy` generated
 * by the epiphany-gcc compiler has some drawbacks.
 * First of all it is stored in external memory, unless you store the full
 * C library (newlib) on the epiphany cores. Secondly it does not do
 * the optimal 8-byte transfers so it is far from optimal.
 *
 * This function resides in local core memory and does 8-byte transfers
 * when possible, meaning if both `dst` and `src` are 8-byte aligned.
 * In other cases, 4-byte or single byte transfers are used.
 */
void ebsp_memcpy(void* dst, const void* src, size_t nbytes);

/**
 * Output a debug message printf style.
 * @param format The formatting string in printf style
 *
 * ebsp_message() outputs a debug message by sending it to shared memory
 * So that the host processor can output it to the terminal
 * The attributes in this definition make sure that the compiler checks the
 * arguments for errors.
 */
void ebsp_message(const char* format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

