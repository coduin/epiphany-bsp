/*
File: host_out_stream.c

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

#include <host_bsp.h>
#include <host_bsp_inspector.h>
#include <stdlib.h>
#include <stdio.h> 
#include <stdint.h> 

int main(int argc, char **argv)
{
    bsp_init("e_out_stream.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    void** outputs = (void**)malloc(sizeof(void*)*bsp_nprocs());
    for (int pid = 0; pid < bsp_nprocs(); pid++)
    {
        outputs[pid] = ebsp_create_up_stream(pid, 21*sizeof(int), 7*sizeof(int));
        *(((int*)outputs[pid])+20)=0xdeadbeaf;  // should not be overwritten
    }

    ebsp_spmd();

    int errorcount = 0;
    int deadbeaferrorcount = 0;
    for (int pid = 0; pid < bsp_nprocs(); pid++)
    {
        if( *(((int*)outputs[pid])+20) != 0xdeadbeaf ) {
            printf("%d has deadbeaf error\n", pid);      
            deadbeaferrorcount++;
        }
        for (int i=0; i<20; i++)
        {
            int output = ((int*) outputs[pid])[i];
            if(output - (20*pid + i)*(20*pid + i) != 0) {
                printf("Error in core %02d: output[%02d] = %d != %d\n", pid, i, output, (20*pid + i)*(20*pid + i));
                errorcount++;
            }
        }
    }
    printf("Only %d cores wrote too many bytes", deadbeaferrorcount); // expect: (Only 0 cores wrote too many bytes)
    printf("Only %d values where wrong", errorcount); // expect: (Only 0 values where wrong)
    
    // finalize
    bsp_end();

    printf("Done"); // expect: (Done)
    return 0;
}

