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
 * Obtain the next chunk of data from a stream.
 *
 * @param address A pointer to a value that is overwritten with the local
 *  memory location of the data chunk
 * @param stream_id The identifier of the stream
 * @param prealloc If this parameter is equal to `1` then the BSP system will
 *  use double buffering, if it is `0` then single buffering is used.
 * @return Number of bytes of the obtained chunk. If stream has
 *  finished or an error has occurred this function will return `0`.
 *
 * @remarks Memory is transferred using the `DMA1` engine.
 * @remarks When using double buffering, the BSP system will allocate memory
 *  for the next chunk, and will start writing to it using the DMA engine
 *  while the current chunk is processed. This requires more (local) memory,
 *  but can greatly increase the overall speed.
 */
int ebsp_move_chunk_down(void** address, unsigned stream_id, int prealloc);

/**
 * Move a chunk of data up from a stream.

 * @param address A pointer to a value that is overwritten with the local
 *  memory location where the next chunk should be written to.
 * @param stream_id The identifier of the stream
 * @param prealloc If this parameter is equal to `1` then the BSP system will
 *  use double buffering, if it is `0` then single buffering is used.
 * @return Number of bytes allocated for the next chunk of this stream. if
 *  stream has finished or an error has occurred this function will return `0`.
 *
 * @remarks Memory is transferred using the `DMA1` engine.
 * @remarks When using the double buffering mode, `*address` will contain the
 *  location of a new chunk of memory, such that the BSP program can continue
 *  while the current chunk is being copied using the DMA engine. This requires
 *  more local memory, but can improve the performance of the program.
 */
int ebsp_move_chunk_up(void** address, unsigned stream_id, int prealloc);

/**
 * Move the cursor pointing to the next chunk to be obtained from a stream.
 *
 * @param stream_id The identifier of the stream
 * @param jump_n_chunks The number of chunks to skip if `jump_n_chunks > 0`,
 *  or to go back if `jump_n_chunks < 0`.
 *
 * @remarks Internally a stream is a collection of data, along with specific
 *  information for a stream. For example, a stream holds a pointer to the
 *  next chunk that should be written to a core. Using this function you can
 *  change which chunk should be the `next` chunk. This allows you to obtain
 *  chunks multiple times, or to skip chunks completely.
 * @remarks This function provides a mechanism through which chunks can be
 *  obtained multiple times. It gives you random access in the memory in
 *  the data stream.
 * @remarks This function has `O(jump_n_chunks)` complexity.
 */
void ebsp_move_down_cursor(int stream_id, int jump_n_chunks);

/**
 * Resets the cursor pointing to the next chunk to be obtained from a stream.
 *
 * @param stream_id The identifier of the stream
 *
 * After calling this function the *next chunk* that is obtained from this
 * stream is equal to the first chunk.
 *
 * @remarks This function has `O(1)` complexity.
 */
void ebsp_reset_down_cursor(int stream_id);

/**
 * Open an up stream.
 *
 * @param address Pointer to a variable that will be overwritten with the
 *  location where the data should be written  for the first chunk that will
 *  be sent up.
 * @param stream_id The identifier of the stream
 * @return Number of bytes that can be written to the first chunk to be sent up.
 *
 * @remarks This function has to be called *before* performing any other operation
 *  on the stream.
 * @remarks A call to the function should always match a single call to
 *  `ebsp_close_up_stream`.
 */
int ebsp_open_up_stream(void** address, unsigned stream_id);

/**
 * Close an up stream.
 *
 * @param stream_id The identifier of the stream
 *
 * Cleans up the stream, and frees any buffers that may have been used by the
 *  stream.
 */
void ebsp_close_up_stream(unsigned stream_id);

/**
 * Open a down stream.
 *
 * @param address Pointer to a variable that will be overwritten with the
 *  location where the data should be written  for the first chunk that will
 *  be sent up.
 * @param stream_id The identifier of the stream
 * @return The size of the first chunk of this stream in bytes.
 *
 * @remarks This function has to be called *before* performing any other operation
 *  on the stream.
 * @remarks A call to the function should always match a single call to
 *  `ebsp_close_down_stream`.
 */
int ebsp_open_down_stream(void** address, unsigned stream_id);

/**
 * Close a down stream.
 *
 * @param stream_id The identifier of the stream
 *
 * Cleans up the stream, and frees any buffers that may have been used by the
 *  stream.
 */
void ebsp_close_down_stream(unsigned stream_id);

/**
 * Set the number of bytes of the current chunk in the up stream.
 *
 * @param stream_id The identifier of the stream
 * @param nbytes The number of bytes that should be moved from the current
 *  chunk of data in the next call to `ebsp_move_chunk_up`.
 */
void ebsp_set_up_chunk_size(unsigned stream_id, int nbytes);

