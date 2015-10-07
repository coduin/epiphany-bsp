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
#include "../common.h"

int main() {
    bsp_begin();
    int s = bsp_pid();
    int p = bsp_nprocs();

    int a = 0;
    bsp_push_reg(&a, sizeof(int));
    bsp_sync();

    int b = 0;
    bsp_push_reg(&b, sizeof(int));
    bsp_sync();

    int c[16] = {0};
    bsp_push_reg(&c, 16 * sizeof(int));
    bsp_sync();

    // first we test puts
    int data = s;
    bsp_hpput((s + 1) % p, &data, &a, 0, sizeof(int));
    bsp_hpput((s + 2) % p, &data, &b, 0, sizeof(int));
    for (int t = 0; t < p; ++t) {
        bsp_hpput(t, &data, &c, sizeof(int) * s, sizeof(int));
    }
    ebsp_barrier();

    // test: can set and get tagsize from core
    EBSP_MSG_ORDERED("%i", a);
    // expect_for_pid: ((pid - 1) % 16)

    // test: can put register multiple vars, and put multiple times
    EBSP_MSG_ORDERED("%i", b);
    // expect_for_pid: ((pid - 2) % 16)

    // test: support for larger variables
    EBSP_MSG_ORDERED("%i", c[5]);
    // expect_for_pid: ("5")

    // next we test gets

    int core_num_next = 0;
    int core_num_next_next = 0;
    bsp_hpget((s + 1) % p, &a, 0, &core_num_next, sizeof(int));
    bsp_hpget((s + 2) % p, &b, 0, &core_num_next_next, sizeof(int));
    bsp_hpget((s + 3) % p, &c, 4 * sizeof(int), &data, sizeof(int));
    ebsp_barrier();

    // test: can set and get tagsize from core
    EBSP_MSG_ORDERED("%i", core_num_next);
    // expect_for_pid: (pid)

    // test: can put register multiple vars, and put multiple times
    EBSP_MSG_ORDERED("%i", core_num_next_next);
    // expect_for_pid: (pid)

    // test: support for larger variables
    EBSP_MSG_ORDERED("%i", data);
    // expect_for_pid: ("4")

    bsp_end();

    return 0;
}
