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

void prepare_descriptor(e_dma_desc_t* desc, void *dst, const void *src, size_t nbytes)
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


void ebsp_dma_push(e_dma_desc_t* desc, void *dst, const void *src, size_t nbytes)
{
    if (nbytes == 0) return;

    // Set the contents of the descriptor
    prepare_descriptor(desc, dst, src, nbytes);
    
    // Change the previous descriptor to chain to this one
    e_dma_desc_t* last = coredata.last_dma_desc;

    unsigned newconfig = (last->config & 0x0000ffff) | ((unsigned)desc << 16) | E_DMA_CHAIN;
    last->config = newconfig;
    coredata.last_dma_desc = desc;

    // Check if the DMA is idle and start it if needed
    volatile unsigned* dmastatus = e_get_global_address(e_group_config.core_row, e_group_config.core_col, (void*)E_REG_DMA1STATUS);
    if ((*dmastatus & 0xf) == 0)
        e_dma_start(desc, E_DMA_1);

    return;
}

void ebsp_dma_wait(e_dma_desc_t* desc)
{
    volatile unsigned* dmastatusreg = e_get_global_address(e_group_config.core_row, e_group_config.core_col, (void*)E_REG_DMA1STATUS);

    int task_in_queue = 1;
    while(task_in_queue)
    {
        unsigned dmastatus = *dmastatusreg;

        // Check if DMA is idle
        if ((dmastatus & 0xf) == 0) return;

        // DMA not idle, so it is working on a descriptor
        e_dma_desc_t* cur = (e_dma_desc_t*)(dmastatus >> 16);

        // Follow path to see if 'desc' still has to be done
        task_in_queue = 0;
        for(;;) {
            if (cur == 0)
                break;
            if (cur == desc) {
                task_in_queue = 1;
                break;
            }
            if ((cur->config & E_DMA_CHAIN) == 0)
                break;
            cur = (e_dma_desc_t*)(cur->config >> 16);
        }
    }

    return;
}

