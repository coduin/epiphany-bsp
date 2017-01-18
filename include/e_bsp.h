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
#include "e_bsp_datatypes.h"
#include "e_bsp_deprecated.h"

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
 * `UINT_MAX/(600000000)` which is roughly 7 seconds.
 *
 * If you want to measure longer time intervals, we suggest you use the
 * (less accurate) ebsp_host_time().
 *
 * @remarks Using this in combination with ebsp_raw_time() leads to unspecified
 * behaviour, you should only use one of these in your program.
 *
 * @remarks This uses the internal Epiphany `E_CTIMER_0` timer so the second
 * timer can be used for other purposes.
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
 * @remarks Using this in combination with bsp_time() leads to unspecified
 * behaviour, you should only use one of these in your program.
 *
 * @remarks This uses the internal Epiphany `E_CTIMER_0` timer so the second
 * timer can be used for other purposes.
 */
unsigned int ebsp_raw_time();

/**
 * Obtain the time in seconds since bsp_begin() was called.
 * @return A floating point value with the number of seconds since bsp_begin()
 *
 * This function uses the system clock of the host to obtain the elapsed time.
 * Because of varying amounts of latency this can be very inaccurate
 * (its precision is in the order of milliseconds), but it supports time
 * intervals of arbitrary length. 
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
 * The order in which the variables are popped does not matter.
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
 * @param packets A pointer to an integer which will be overwritten with the
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
 * @param status A pointer to an integer receiving the message data size
 * in bytes.
 * @param tag A pointer to a buffer receiving the message tag
 *
 * Upon return, the integer pointed to by status will receive the size of the
 * data payload in bytes of the next message in the queue. If there is no
 * next message it will be set to -1.
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
 * Open a stream that was created using `bsp_stream_create` on the host.
 *
 * @param stream Pointer to an existing `bsp_stream` struct to hold the stream
 * data. This struct can be allocated on the stack by the user.
 * @param stream_id The index of the stream.
 * @return Nonzero if succesful.
 *
 * The first stream created by the host will have `stream_id` 0.
 *
 * Usage example:
 * \code{.c}
 * bsp_stream mystream;
 * if( bsp_stream_open(&mystream, 3) ) {
 *     // Get some data
 *     void* buffer = 0;
 *     bsp_stream_move_down(&mystream, &buffer, 0);
 *     // The data is now in buffer
 *     // Finally, close the stream
 *     bsp_stream_close(&mystream);`
 * }
 * \endcose
 *
 * @remarks This function has to be called *before* performing any other
 * operation on the stream.
 * @remarks A call to the function should always match a single call to
 *  `bsp_stream_close`.
 */
int bsp_stream_open(ebsp_stream* stream, int stream_id);

/**
 * Wait for pending transfers to complete and close a stream.
 *
 * @param stream The handle of the stream, opened by `bsp_stream_open`.
 *
 * Behaviour is undefined if `stream` is not a handle opened by
 * `bsp_stream_open`.
 *
 * Cleans up the stream, and frees any buffers that may have been used by the
 * stream.
 */
void bsp_stream_close(ebsp_stream* stream);

/**
 * Move the cursor in the stream, to change the next token to be obtained.
 *
 * @param stream The handle of the stream
 * @param delta_tokens The number of tokens to skip if `delta_tokens > 0`,
 *  or to go back if `delta_tokens < 0`.
 *
 * If `delta_tokens` is out of bounds, then the cursor will be moved to
 * the start or end of the stream respectively.
 * `bsp_stream_seek(i, INT_MIN)` will set the cursor to the start
 * `bsp_stream_seek(i, INT_MAX)` will set the cursor to the end of the stream
 * 
 * Note that if `bsp_stream_move_down` is used with `preload` enabled
 * (meaning the last call to that function had `preload` enabled),
 * then the preloaded token will not be changed, so the first call to
 * `bsp_stream_move_down` after this will still yield a token from the
 * previous position.
 * If `preload` was not enabled then the next call to `bsp_stream_move_down`
 * will yield a token from the new position.
 *
 * @remarks This function provides a mechanism through which chunks can be
 *  obtained multiple times. It gives you random access in the memory in
 *  the data stream.
 * @remarks This function has `O(delta_tokens)` complexity.
 */
void bsp_stream_seek(ebsp_stream* stream, int delta_tokens);

/**
 * Obtain the next token from a stream.
 *
 * @param stream The handle of the stream
 * @param buffer Receives a pointer to a local copy of the next token.
 * @param preload If this parameter is nonzero then the BSP system will
 * preload the next token asynchroneously (double buffering).
 * @return Number of bytes of the obtained chunk. If stream has
 *  finished or an error has occurred this function will return `0`.
 *
 * @remarks Behaviour is undefined if the stream was not opened using
 * `bsp_stream_open`.
 * @remarks Memory is transferred using the `DMA1` engine.
 * @remarks When using double buffering, the BSP system will allocate memory
 *  for the next chunk, and will start writing to it using the DMA engine
 *  while the current chunk is processed. This requires more (local) memory,
 *  but can greatly increase the overall speed.
 */
int bsp_stream_move_down(ebsp_stream* stream, void** buffer, int preload);

/**
 * Write a local token up to a stream.
 *
 * @param stream The handle of the stream
 * @param data The data to be sent up the stream
 * @param data_size The size of the data to be sent, i.e. the size of the token.
 * Behaviour is undefined if it is not a multiple of 8.
 * If it is not a multiple of 8 bytes then transfers will be slow.
 * @param wait_for_completion If nonzero this function blocks untill
 * the data is completely written to the stream.
 * @return Number of bytes written. Zero if an error has occurred.
 *
 * The function *always* waits for the previous token to have finished.
 *
 * If `wait_for_completion` is nonzero, this function will wait untill
 * the data is transferred. This corresponds to single buffering.
 *
 * Alternativly, double buffering can be used as follows.
 * Set `wait_for_completion` to zero and continue constructing the next token
 * in a different buffer. Usage example:
 * \code{.c}
 * int* buf1 = ebsp_malloc(100 * sizeof(int));
 * int* buf2 = ebsp_malloc(100 * sizeof(int));
 * int* curbuf = buf1;
 * int* otherbuf = buf2;
 *
 * ebsp_stream s;
 * bsp_stream_open(&s, 0); // open stream 0
 * while (...) {
 *     // Fill curbuf
 *     for (int i = 0; i < 100; i++)
 *         curbuf[i] = 5;
 *     
 *     // Send up
 *     bsp_stream_move_up(&s, curbuf, 100 * sizeof(int), 0);
 *     // Use other buffer
 *     swap(curbuf, otherbuf);
 * }
 * ebsp_free(buf1);
 * ebsp_free(buf2);
 * \endcode
 *
 * @remarks Behaviour is undefined if the stream was not opened using
 * `bsp_stream_open`.
 * @remarks Memory is transferred using the `DMA1` engine.
 */
int bsp_stream_move_up(ebsp_stream* stream, const void* data, int data_size, int wait_for_completion);

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
 * Push a new task to the DMA engine. See the documentation on Memory
 * Management for details on the DMA engine.
 * @param desc   Used in combination with ebsp_dma_wait(). Should be seen
 * as a *handle* to the task. Its contents are populated by this function.
 * @param dst    Destination address
 * @param src    Source address
 * @param nbytes Amount of bytes to be copied
 *
 * \verbatim embed:rst:leading-asterisk
 * .. warning::
 *     Source ``src`` and destination ``dst`` can **NOT** be both on the local
 *     core. Doing so results in undefined behaviour. At least one of the two
 *     addresses should be on another core or in external memory.
 * \endverbatim
 *
 * Assumes previous task in `desc` is completed (use ebsp_dma_wait())
 *
 * The DMA (`E_DMA_1`) will be started if it was not started yet.
 * If it was already started, this task will be pushed to a queue so that it
 * will be done some time later. Use ebsp_dma_wait() to wait for the task to
 * complete.
 *
 * Usage example:
 * \code{.c}
 * int s = bsp_pid();
 * int p = bsp_nprocs();
 * 
 * // Data to be sent
 * float mydata[16];

 * // Buffer to receive data
 * float incomingdata[16];

 * // Register it in the BSP system
 * bsp_push_reg(&incomingdata, sizeof(incomingdata));
 * bsp_sync();
 * 
 * // Get an address for the incomingdata buffer on the core with pid s + 1.
 * float* remotedata = ebsp_get_direct_address((s+1)%p, &incomingdata);
 * 
 * // Start the DMA to copy the data from mydata on this core to incomingdata on the next core
 * ebsp_dma_handle descriptor;
 * ebsp_dma_push(&descriptor, remotedata, &mydata, sizeof(mydata));
 * 
 * // Do lengthy computation
 * do_computations();
 * 
 * // Wait for the DMA transfer to finish
 * ebsp_dma_wait(&descriptor);
 * 
 * // Done
 * \endcode

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
 * This function blocks untill the task in `desc` is completed.
 * Use somewhere after ebsp_dma_push(). See ebsp_dma_push() for example code.
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
void* ebsp_get_direct_address(int pid, const void* variable);

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

