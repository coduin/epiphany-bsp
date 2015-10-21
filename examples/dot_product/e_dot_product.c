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

int main() {
    bsp_begin();

    int p = bsp_pid();

    int chunk = 0;
    int* a = 0, * b = 0;

    int packets, accum_bytes, status, tag;
    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++) {
        bsp_get_tag(&status, &tag);
        if (status == -1)
            break;

        void* buffer = ebsp_malloc(status);
        if (buffer == 0) {
            ebsp_message("Could not allocate memory");
            break;
        }

        bsp_move(buffer, status);

        if (tag == 1) {
            chunk = *(int*)buffer;
        } else if (tag == 2) {
            a = (int*)buffer;
            continue;
        } else if (tag == 3) {
            b = (int*)buffer;
            continue;
        }
        ebsp_free(buffer);
    }

    int sum = 0;

    if (a == 0 || b == 0) {
        ebsp_message("Did not receive data from host");
    } else {
        for (int i = 0; i < chunk; ++i)
            sum += a[i] * b[i];
    }

    if (a) ebsp_free(a);
    if (b) ebsp_free(b);

    // A sync is required between getting messages
    // from host and sending them back
    bsp_sync();

    tag = p;
    ebsp_send_up(&tag, &sum, sizeof(int));

    bsp_end();

    return 0;
}
