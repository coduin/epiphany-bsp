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

#include "e_bsp_private.h"

#define local_mask (0xfff00000)
extern unsigned dma_data_size[8];

void _prepare_descriptor(e_dma_desc_t* desc, void* dst, const void* src,
                         size_t nbytes) {
    // Alignment
    unsigned index =
        (((unsigned)dst) | ((unsigned)src) | ((unsigned)nbytes)) & 7;
    unsigned shift = dma_data_size[index] >> 5;

    desc->config =
        E_DMA_MASTER | E_DMA_ENABLE | E_DMA_IRQEN | dma_data_size[index];
    if ((((unsigned)dst) & local_mask) == 0)
        desc->config |= E_DMA_MSGMODE;
    desc->inner_stride = 0x00010001 << shift;
    desc->count = 0x00010000 | (nbytes >> shift);
    desc->outer_stride = 0;
    desc->src_addr = (void*)src;
    desc->dst_addr = (void*)dst;
}

void ebsp_dma_push(ebsp_dma_handle* descriptor, void* dst, const void* src,
                   size_t nbytes) {
    if (nbytes == 0)
        return;

    e_dma_desc_t* desc = (e_dma_desc_t*)descriptor;

    // Set the contents of the descriptor
    _prepare_descriptor(desc, dst, src, nbytes);

    // Take the end of the current descriptor chain
    e_dma_desc_t* last = coredata.last_dma_desc;

    if (last == NULL) {
        // No current chain, replace it by this one
        coredata.last_dma_desc = desc;
    } else if (last != desc) {
        // We need to disable interrupts because
        // if the interrupt fires between `newconfig = ...`
        // and `last->config = ...` then the interrupt
        // clears the E_DMA_ENABLE bit but it is then set again
        // by this code below, which is not what we want
        e_irq_global_mask(E_TRUE);
        // Attach desc to last
        unsigned newconfig =
            (last->config & 0x0000ffff) | ((unsigned)desc << 16);
        last->config = newconfig;
        e_irq_global_mask(E_FALSE);
        coredata.last_dma_desc = desc;
    }

    // In principle it could happen that at this point, the previous
    // DMA task finished, its interrupt fires which starts the DMA
    // engine for THIS task, which could then ALSO finish.
    // Therefore we have to check for this inside the if statement below

    // Start DMA if not started yet
    if (coredata.cur_dma_desc == 0) {
        if (desc->config & E_DMA_ENABLE) {
            // Start the DMA engine using the kickstart bit
            coredata.cur_dma_desc = desc;
            unsigned kickstart = ((unsigned)desc << 16) | E_DMA_STARTUP;
            *coredata.dma1config = kickstart;
        }
    }
}

void __attribute__((interrupt)) _dma_interrupt() {
    // If DMA is in chaining mode, an interrupt will be fired after a chain
    // element is completed. At this point in the interrupt, the DMA will
    // already be busy doing the next element of the chain or even the one
    // after that if it fired two interrupts really quickly after each other.
    //
    // The DMA should not be used in chaining mode, or the whole method
    // does not work. It seems like the codeblock below is a solution
    // but it fails when there are many tiny dma transfers in the chain
    // and the DMA fires many interrupts that will be lost in obliviion
    // because this function is still running.
    // We can not lose any interrupts, because we need to advance the
    // `cur_dma_desc` pointer at each interrupt, even for chains
    //
    // unsigned status = *coredata.dma1status;
    // if (status & 0xf)
    //     return;

    // Grab the current task
    e_dma_desc_t* desc = coredata.cur_dma_desc;
    if (desc == 0) { // should not happen
        // We want to show an error message but not using
        // ebsp_message because we are inside an interrupt.
        // Instead we use the following flag that the host reads.
        combuf->interrupts[coredata.pid] =
            0x80; // Use (1 << E_DMA1_INT) as error message
        return;
    }

    // Mark 'desc' as finished
    desc->config &= ~(E_DMA_ENABLE);

    // Go to the 'next' task
    e_dma_desc_t* next = (e_dma_desc_t*)(desc->config >> 16);
    coredata.cur_dma_desc = next;

    if (next) {
        // Start the DMA engine using the kickstart bit
        unsigned kickstart = ((unsigned)next << 16) | E_DMA_STARTUP;
        *coredata.dma1config = kickstart;
    }
}

void ebsp_dma_wait(ebsp_dma_handle* descriptor) {
    volatile unsigned* config = &descriptor->config;
    while (*config & E_DMA_ENABLE) {
    }
}

