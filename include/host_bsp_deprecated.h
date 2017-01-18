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
 * Creates a down stream
 *
 * @param src The data which should be streamed down to an Epiphany core.
 * @param dst_core_id The processor identifier of the receiving core.
 * @param nbytes The total number of bytes of the data to be streamed down.
 * @param chunksize The size in bytes of a single chunk. Must be at least 16.
 *
 * This function outputs an error if `chunksize` is less than 16.
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
 *  sent up through this stream. Must be at least 16.
 * @return A pointer to a section of external memory storing the chunks
 *  sent up by the sending core.
 *
 * This function outputs an error if `chunksize` is less than 16.
 */
void* ebsp_create_up_stream(int dst_core_id, int max_nbytes, int chunksize);
