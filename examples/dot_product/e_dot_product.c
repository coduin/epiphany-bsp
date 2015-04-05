/*
File: e_dot_product.c

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

int main()
{
    bsp_begin();

    int p = bsp_pid();

    int chunk = 0;
    int *a = 0, *b = 0;

    char buffer[2048];
    int offset = 0;

    int packets, accum_bytes, status, tag;
    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++)
    {
        // We assume all packet sizes are multiples of 4
        // If not, the cores will crash because of unaligned memory accesses
        bsp_get_tag(&status, &tag);
        bsp_move(&buffer + offset, sizeof(buffer)-offset);
        offset += status;

        if (tag == 1) chunk = *(int*)(&buffer + offset);
        else if (tag == 2) a = (int*)(&buffer + offset);
        else if (tag == 3) b = (int*)(&buffer + offset);
    }

    int sum = 0;
    for (int i = 0; i < chunk; ++i)
        sum += a[i] * b[i];

    tag = p;
    ebsp_send_up(&tag, &sum, sizeof(int));

    bsp_end();

    return 0;
}
