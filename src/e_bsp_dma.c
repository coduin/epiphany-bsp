/*
File: e_bsp_dma.c

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

#include "e_bsp_private.h"

#define local_mask (0xfff00000)
extern unsigned dma_data_size[8];

void _prepare_descriptor(e_dma_desc_t* desc, void *dst, const void *src, size_t nbytes)
{
    // Alignment
    unsigned index = (((unsigned) dst) | ((unsigned) src) | ((unsigned) nbytes)) & 7;
    unsigned shift = dma_data_size[index] >> 5;

    desc->config = E_DMA_MASTER | E_DMA_ENABLE | dma_data_size[index];
    if ((((unsigned)dst) & local_mask) == 0)
        desc->config |= E_DMA_MSGMODE;
    desc->inner_stride  = 0x00010001 << shift;
    desc->count         = 0x00010000 | (nbytes >> shift);
    desc->outer_stride  = 0;
    desc->src_addr      = (void*)src;
    desc->dst_addr      = (void*)dst;
}


void ebsp_dma_push(ebsp_dma_handle* descriptor, void *dst, const void *src, size_t nbytes)
{
    if (nbytes == 0)
        return;

    e_dma_desc_t* desc = (e_dma_desc_t*)descriptor;

    // Set the contents of the descriptor
    _prepare_descriptor(desc, dst, src, nbytes);

    // Change the previous descriptor to chain to this one if it is a different one
    e_dma_desc_t* last = (e_dma_desc_t*)coredata.last_dma_desc;

    if (last == NULL) {
        coredata.last_dma_desc = descriptor;
    }
    else if (last != desc) {
        unsigned newconfig = (last->config & 0x0000ffff) | ((unsigned)desc << 16) | E_DMA_CHAIN;
        last->config = newconfig;
        coredata.last_dma_desc = descriptor;
    }
}

void ebsp_dma_start(ebsp_dma_handle* desc)
{
    // Check if the DMA is idle and start it if needed
    volatile unsigned* dmastatus = e_get_global_address(e_group_config.core_row, e_group_config.core_col, (void*)E_REG_DMA1STATUS);
    if ((*dmastatus & 0xf) == 0)
        e_dma_start((e_dma_desc_t*)desc, E_DMA_1);
}

void ebsp_dma_wait(ebsp_dma_handle* descriptor)
{
    e_dma_desc_t* desc = (e_dma_desc_t*)descriptor;
    volatile unsigned* dmastatusreg = e_get_global_address(e_group_config.core_row, e_group_config.core_col, (void*)E_REG_DMA1STATUS);

    // There is a chain of DMA tasks. This function loops until 'descriptor'
    // is no longer one of the elements of this chain

    int task_in_queue = 1;
    while(task_in_queue)
    {
        unsigned dmastatus = *dmastatusreg;

        // Check if DMA is idle, i.e. empty chain
        if ((dmastatus & 0xf) == 0)
            return;

        // DMA not idle, so it is working on a descriptor
        e_dma_desc_t* cur = (e_dma_desc_t*)(dmastatus >> 16);

        // Follow path to see if 'desc' is in the chain
        task_in_queue = 0;
        for(;;) {
            // This should not happen
            if (cur == 0)
                break;

            // Found the task. This means we have to wait
            if (cur == desc) {
                task_in_queue = 1;
                break;
            }

            // End of chain: there is no next task
            if ((cur->config & E_DMA_CHAIN) == 0)
                break;

            cur = (e_dma_desc_t*)(cur->config >> 16);
        }
    }
}

