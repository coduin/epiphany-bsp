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

int main() {
    bsp_begin();

    int p = bsp_pid();

    int prime_begin = -1;
    int prime_end = -1;

    char buffer[2 * sizeof(int)]; // get prime_begin and prime_end
    void* ptr = (void*)&buffer;
    int sizeleft = sizeof(buffer);

    int packets, accum_bytes, status, tag;
    bsp_qsize(&packets, &accum_bytes);
    for (int i = 0; i < packets; i++) {
        // We assume all packet sizes are multiples of 4
        // If not, the cores will crash because of unaligned memory accesses
        bsp_get_tag(&status, &tag);
        bsp_move(ptr, sizeleft);

        if (tag == 1)
            prime_begin = *(int*)ptr;
        else if (tag == 2)
            prime_end = *(int*)ptr;

        sizeleft -= status;
        ptr += status;
    }

    void* pre_primes = 0;

    int prime_table_size = 3600;
    char* is_prime = ebsp_malloc(prime_table_size * sizeof(char));

    int prime_cursor_begin = prime_begin;
    int prime_cursor_end = prime_begin + prime_table_size;

    int out_stream_size = -1;
    void* out_stream_end = 0;
    void* out_stream = 0;

    int n_primes_found = 0;

    int in_stream_id = 0;
    int out_stream_id = 1;

    while (1) {
        if (prime_cursor_begin >= prime_end)
            break;

        if (prime_cursor_end > prime_end)
            prime_cursor_end = prime_end;

        for (int i = 0; i < prime_table_size; i++)
            is_prime[i] = 1;

        int chunk_size = -1;

        while ((chunk_size =
                    ebsp_get_next_chunk(&pre_primes, in_stream_id, 0)) != 0) {
            for (unsigned offset = 0; offset < chunk_size;
                 offset += sizeof(int)) {
                int prime_i = *((int*)((unsigned)pre_primes + offset));
                for (int j = (prime_cursor_begin + prime_i - 1) / prime_i;
                     j * prime_i < prime_cursor_end; j++) {
                    is_prime[j * prime_i] = 0;
                }
            }
        }
        ebsp_reset_in_cursor(in_stream_id);

        for (int p = prime_cursor_begin; p < prime_cursor_end; p++) {
            if (out_stream == out_stream_end) {
                out_stream_size = ebsp_write_out(&out_stream, out_stream_id, 1);
                out_stream_end = out_stream + out_stream_size;
            }

            if (is_prime[p]) {
                *((int*)out_stream) = p;
                out_stream += sizeof(int);
                n_primes_found++;
            }
        }

        prime_cursor_begin = prime_cursor_end + prime_table_size;
        prime_cursor_end = prime_cursor_begin + prime_table_size;
    }
    int nbytes_left = out_stream_size - (out_stream_end - out_stream);
    ebsp_set_out_size(1, nbytes_left);
    ebsp_write_out(&out_stream, out_stream_id, 1);

    bsp_sync();

    tag = p;
    ebsp_send_up(&tag, &n_primes_found, sizeof(int));

    bsp_end();

    return 0;
}
