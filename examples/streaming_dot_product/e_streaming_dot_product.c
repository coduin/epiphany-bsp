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

#include <e_bsp.h>
#include <common.h>

int main() {
    bsp_begin();

    int p = bsp_pid();
    int sum = 0;
    void* a = 0;
    void* b = 0;

    int a_size;
    int b_size;

    ebsp_open_down_stream((void**)&a, 0);
    ebsp_open_down_stream((void**)&b, 1);

    while (1) {

        a_size = ebsp_move_chunk_down(&a, 0, 1);
        b_size = ebsp_move_chunk_down(&b, 1, 1);

        if (a_size != b_size) {
            ebsp_message("mismatching chunks!");
            ebsp_message("b_size = %d", b_size);
        }

        if (a_size == 0)
            break;

        for (unsigned offset = 0; offset < a_size; offset += sizeof(int)) {
            int ai = *((int*)((unsigned)a + offset));
            int bi = *((int*)((unsigned)b + offset));
            sum += ai * bi;
        }
    }

    bsp_sync();

    int tag = p;
    ebsp_send_up(&tag, &sum, sizeof(int));

    ebsp_close_down_stream(0);
    ebsp_close_down_stream(1);

    bsp_end();

    return 0;
}
