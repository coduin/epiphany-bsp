/*
File: e_streaming_dot_product.c

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

#include <e_bsp.h>
#include <common.h>

int main()
{
    bsp_begin();

    int p = bsp_pid();
    int sum = 0;
    void* a = 0;
    void* b = 0;
    int counter = 0;

    ebsp_message("the loop");
    while (1){
        int a_size = get_next_chunk(&a, 0, 1);
        int b_size = get_next_chunk(&b, 1, 1);
       
        if (a_size != b_size) {
            ebsp_message("mismatching chunks!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            ebsp_message("b_size = %d", b_size);
        }

        ebsp_message("a_size = %d", a_size);

        if (a_size == 0)
            break;

        for (unsigned offset = 0; offset < a_size; offset += sizeof(int) )
        {
            int ai = *((int*)((unsigned)a + offset));
            int bi = *((int*)((unsigned)b + offset));
            counter += ai*bi;
        }
        ebsp_message("counter = %d", counter);
    }

    ebsp_message("done counting!");
    // A sync is required between getting messages
    // from host and sending them back
    bsp_sync();

    int tag = p;
    ebsp_send_up(&tag, &sum, sizeof(int));

    bsp_end();

    return 0;
}
