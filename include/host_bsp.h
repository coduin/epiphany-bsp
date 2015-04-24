/*
File: bsp_host.h

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
 * @return 1 on success, 0 on failure
 *
 * This sets up all the BSP variables and loads the epiphany BSP program.
 *
 *  e_name: A string containing the name of the eBSP program.
 *  argc: An integer containing the number of input arguments
 *  argv: An array of strings containg the input flags.
 */
int bsp_init(const char* e_name, int argc, char **argv);

/** Set the callback for syncing
 */
void ebsp_set_sync_callback(void (*cb)());

/** Set the callback for finalizing
 */
void ebsp_set_end_callback(void (*cb)());

/** Starts the SPMD program on the Epiphany cores.
 *  flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int ebsp_spmd();

/** Starts the BSP program.
 *
 *  nprocs: An integer indicating the number of processors to run on.
 *  flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int bsp_begin(int nprocs);

/** Finalizes and cleans up the BSP program.
 *  flag: An integer indicating whether the function finished
 *                succesfully, in which case it is 1, or 0 otherwise.
 */
int bsp_end();

/** Returns the number of available processors.
 *
 *  nprocs: An integer indicating the number of available processors.
 */
int bsp_nprocs();

/**
 * Set initial tagsize.
 *
 * Should be called at most once, before any messages are sent.
 * Calling this when receiving messages results in undefined behaviour.
 */
void ebsp_set_tagsize(int *tag_bytes);

/**
 * Send initial messages
 */
void ebsp_send_down(int pid, const void *tag, const void *payload, int nbytes);

/**
 * Get the tag-size as set by the epiphany cores.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
int ebsp_get_tagsize();

/**
 * Get the amount of messages in the queue and their total size
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_qsize(int *packets, int *accum_bytes);

/**
 * Peek the next message.
 *
 * Upon return, status holds the amount of bytes of the next message payload,
 * or -1 if there are no more messages.
 * tag will hold the tag of the next message. The buffer pointed to by tag
 * should be large enough (ebsp_get_tagsize).
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_get_tag(int *status, void *tag);

/**
 * Get the next message and pop it from the queue.
 *
 * Upon return, payload will hold the contents of the message.
 * The buffer will only be filled till at most buffer_size. Remaining
 * data is truncated. Use ebsp_get_tag to get the size of the data payload.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
void ebsp_move(void *payload, int buffer_size);

/**
 * Get the pointer to the next message payload, and pop the message.
 * This will be a pointer to external memory so it is slow.
 *
 * Use only for gathering result messages at the end of a BSP program.
 */
int ebsp_hpmove(void **tag_ptr_buf, void **payload_ptr_buf);

