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

    int fast_mode = 1;
    int out_stream_size = -1;
    void* out_stream_end = 0;
    void* out_stream = 0;
    for(int i=20*p; i<20*(p+1); i++) {
        if(out_stream == out_stream_end) {
            out_stream_size = ebsp_write_out(&out_stream, 0, fast_mode);
            out_stream_end = out_stream + out_stream_size;
        }
        *((int*)out_stream) = i*i;
        out_stream += sizeof(int);
    }

    int nbytes_left = out_stream_size-(out_stream_end-out_stream);
    ebsp_set_out_size(0, nbytes_left);
    ebsp_write_out(&out_stream, 0, fast_mode);

    bsp_end();

    return 0;
}

