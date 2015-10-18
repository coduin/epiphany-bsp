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

/**
 * @file host_bsp.h
 * @brief All BSP core functions for the ARM host processor.
 *
 * This file contains all BSP core functions for the ARM host processor.
 *
 * ## BSP Message Passing
 *
 * See e_bsp.h for more information about BSP message passing.
 * The following information is specific for sending messages between the
 * host and Epiphany processor.
 *
 * The functions listed below can be used to send messages to message queue
 * of the programs for initialization and retrieve messages to gather results.
 *
 * The initialization messages will only remain in the queue until bsp_sync()
 * has been called for the first time by the Epiphany program.
 *
 * The default tagsize is zero.
 *
 * Sending messages must be done after bsp_init()
 * Retrieving messages must be done before bsp_end()
 *
 * - ebsp_set_tagsize()
 * - ebsp_send_down()
 * - ebsp_get_tagsize()
 * - ebsp_qsize()
 * - ebsp_get_tag()
 * - ebsp_move()
 * - ebsp_hpmove()
 */

#pragma once
#include <e-hal.h>

/**
 * Write data to the Epiphany processor.
 * @param pid The pid of the target processor
 * @param src A pointer to the source data
 * @param dst The destination address (as seen by the Epiphany core)
 * @param size The amount of bytes to be copied
 * @return 1 on success, 0 on failure
 *
 * This is an alternative to the BSP Message Passing system.
 */
int ebsp_write(int pid, void* src, off_t dst, int size);

/**
 * Read data from the Epiphany processor.
 * @param pid The pid of the source processor
 * @param src The source address (as seen by the Epiphany core)
 * @param dst A pointer to a buffer receiving the data
 * @param size The amount of bytes to be copied
 * @return 1 on success, 0 on failure
 *
 * This is an alternative to the BSP Message Passing system.
 */
int ebsp_read(int pid, off_t src, void* dst, int size);

/**
 * Initializes the BSP system.
 * @param e_name A string with the srec binary name of the Epiphany program
 * @param argc The number of input arguments
 * @param argv An array of strings with the input arguments
 * @return 1 on success, 0 on failure
 *
 * Sets up all the BSP variables and loads the epiphany BSP program.
 *
 * The string `e_name` must be of the form `myprogram.srec`. This function
 * will search for the file in the same directory as the host program,
 * and not in the current working directory.
 *
 * Usage example:
 * \code{.c}
 * int main(int argc, char** argv)
 * {
 *     bsp_init("e_program.srec", argc, argv);
 *     ...
 *     return 0;
 * }
 * \endcode
 *
 * @remarks The `argc` and `argv` parameters are ignored in the current
 * implementation.
 */
int bsp_init(const char* e_name, int argc, char** argv);

/**
 * Set the (optional) callback for synchronizing epiphany cores with the
 * host program.
 * @param cb A function pointer to the callback function
 *
 * This callback is called when all Epiphany cores have called
 * ebsp_host_sync(). Note that this does not happen at bsp_sync().
 */
void ebsp_set_sync_callback(void (*cb)());

/**
 * Set the (optional) callback for finalizing.
 * @param cb A function pointer to the callback function
 *
 * This callback is called when ebsp_spmd() finishes. It is primarily used
 * by the ebsp memory inspector and should not be needed.
 */
void ebsp_set_end_callback(void (*cb)());

/**
 * Runs the Epiphany program on the Epiphany cores.
 * @return 1 on success, 0 on failure
 *
 * This function will block untill the BSP program is finished.
 */
int ebsp_spmd();

/**
 * Loads the BSP program onto the Epiphany cores.
 * @param nprocs The number of processors to run on
 * @return 1 on success, 0 on failure
 *
 * Usage example:
 * \code{.c}
 * int main(int argc, char** argv)
 * {
 *     bsp_init("e_program.srec", argc, argv);
 *     bsp_begin(bsp_nprocs());
 *     ...
 *     return 0;
 * }
 * \endcode
 *
 * @remarks The current implementation only allows `nprocs` to be a multiple
 * of 4 on the 16-core Parallella. Other values of `nprocs` are rounded down.
 */
int bsp_begin(int nprocs);

/**
 * Finalizes and cleans up the BSP program.
 * @return 1 on success, 0 on failure
 *
 * Usage example:
 * \code{.c}
 * int main(int argc, char** argv)
 * {
 *     bsp_init("e_program.srec", argc, argv);
 *     bsp_begin(bsp_nprocs());
 *     ebsp_spmd();
 *     bsp_end();
 *     return 0;
 * }
 * \endcode
 *
 * @remarks This function is different from the bsp_end function in e_bsp.h
 */
int bsp_end();

/**
 * Returns the number of available processors (Epiphany cores).
 * @return The number of available processors
 *
 * This function may be called after bsp_init().
 */
int bsp_nprocs();

/**
 * Set initial tagsize for message passing.
 * @param tag_bytes A pointer to an integer containing the new tagsize,
 * receiving the old tagsize on return.
 *
 * The default tagsize is zero.
 * This function should be called at most once, before any messages are sent.
 * Calling this when receiving messages results in undefined behaviour.
 *
 * It is not possible to send messages with different tag sizes. Doing so
 * will result in undefined behaviour.
 *
 * @remarks The tagsize set using this function is also used for inter-core
 * messages.
 */
void ebsp_set_tagsize(int* tag_bytes);

/**
 * Send a message to the Epiphany cores.
 * @param pid The pid of the target processor
 * @param tag A pointer to the message tag
 * @param payload A pointer to the data payload
 * @param nbytes The size of the payload in bytes
 *
 * This is the preferred way to send initial data (for computation) to the
 * Epiphany cores.
 *
 * The size of the buffer pointed to by tag has to be `tagsize`, and must be
 * the same for every message being sent.
 */
void ebsp_send_down(int pid, const void* tag, const void* payload, int nbytes);

/**
 * Get the tagsize as set by the Epiphany program.
 * @return The tagsize in bytes
 *
 * Use only for gathering result messages at the end of a BSP program.
 *
 * When ebsp_spmd() returns, the Epiphany program can have set a different
 * tagsize which can be obtained using this function.
 */
int ebsp_get_tagsize();

/**
 * Get the amount of messages in the queue and their total size in bytes.
 * @param packets A pointer to an integer receiving the number of messages
 * @param accum_bytes The total size of the data payloads of the messages,
 * in bytes.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_qsize(int* packets, int* accum_bytes);

/**
 * Peek the next message.
 * @param status A pointer to an integer receiving the amount of bytes of the
 * next message payload, or -1 if there are no more messages.
 * @param tag A pointer to a buffer receiving the tag of the next message.
 * This buffer should be large enough (ebsp_get_tagsize()).
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_get_tag(int* status, void* tag);

/**
 * Get the next message from the message queue and pop the message.
 * @param payload A pointer to a buffer receiving the data payload
 * @param buffer_size The size of the buffer
 *
 * This will copy the payload and pop the message from the queue.
 * The size of the payload can be obtained by calling bsp_get_tag().
 * If `buffer_size` is smaller than the data payload then the data is
 * truncated.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_move(void* payload, int buffer_size);

/**
 * Get the next message, with tag, from the queue and pop the message.
 * @param tag_ptr_buf A pointer to a pointer receiving the location of the tag
 * @param payload_ptr_buf A pointer to a pointer receiving the location of the
 * data pyaload
 * @return The number of bytes of the payload data
 *
 * This is the faster alternative of ebsp_move(), as this function does
 * not copy the data but returns the pointers to it.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
int ebsp_hpmove(void** tag_ptr_buf, void** payload_ptr_buf);

/**
 * Creates a down stream
 *
 * @param src The data which should be streamed down to an Epiphany core.
 * @param dst_core_id The processor identifier of the receiving core.
 * @param nbytes The total number of bytes of the data to be streamed down.
 * @param chunksize The size in bytes of a single chunk.
 *
 * @remarks The data is copied from `src`, such that the data `src` can be
 *  safely freed or overwritten after this call.
 */

void ebsp_create_down_stream(const void* src, int dst_core_id, int nbytes,
                             int chunksize);

/**
 * Creates an up stream
 *
 * @param dst_core_id The processor identifier of the sending core.
 * @param max_nbytes The maximum number of bytes that will be sent up using
 *  this up stream.
 * @param chunksize The maximum number of bytes of a single chunk that can be
 *  sent up through this stream.
 */
void* ebsp_create_up_stream(int dst_core_id, int max_nbytes, int chunksize);
