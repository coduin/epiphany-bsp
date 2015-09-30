/*
File: e_out_stream.c

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

int main() {
    bsp_begin();

    // int s = bsp_pid();

    float* upstream = 0;
    int chunk_size = ebsp_open_up_stream((void**)&upstream, 0);
    int chunks = 4;

    float* downchunk;
    float* downchunkB;

    ebsp_open_down_stream((void**)&downchunk, 1);
    ebsp_open_down_stream((void**)&downchunkB, 2);

    for (int i = 0; i < chunks; ++i) {
        ebsp_move_chunk_down((void**)&downchunk, 1, 0);
        ebsp_move_chunk_down((void**)&downchunkB, 2, 0);
        for (int j = 0; j < chunk_size / sizeof(float); ++j) {
            upstream[j] = (i % 2 == 1) ? downchunk[j] : downchunkB[j];
        }
        ebsp_move_chunk_up((void**)&upstream, 0, 0);
    }

    ebsp_close_up_stream(0);
    ebsp_close_down_stream(1);
    ebsp_close_down_stream(2);

    bsp_end();

    return 0;
}

