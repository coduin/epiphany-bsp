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

typedef struct {
    unsigned config;
    unsigned inner_stride;
    unsigned count;
    unsigned outer_stride;
    void* src_addr;
    void* dst_addr;
} __attribute__((aligned(8))) ebsp_dma_handle;

typedef struct {
    ebsp_dma_handle e_dma_desc; // descriptor of dma, used as dma_id as well
    void* cursor;               // current position of the stream in extmem
    int32_t id;                 // stream_id of the stream
    void* extmem_start;         // extmem data in e_core address space
    void* extmem_end;           // end of allocated region
    void* current_buffer;       // pointer (in e_core_mem) to current chunk
    void* next_buffer;          // pointer (in e_core_mem) to next chunk
    uint32_t max_chunksize; // maximum size of a token exluding 8 byte header
} __attribute__((aligned(8))) ebsp_stream;


