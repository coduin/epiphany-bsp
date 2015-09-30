/*
File: e_bsp_nprocs.c

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
    float t_old, t_new;
    int backward;

    t_old = bsp_time();
    backward = 0;
    for (int i = 0; i < 30; i++) {
        bsp_sync();
        t_new = bsp_time();
        if (t_new <= t_old) {
            backward = 1;
            ebsp_message("Time runs backwards? %f->%f",t_old, t_new);
        }
        t_old = t_new;
    }
    if (backward == 0 && bsp_pid() == 0)
        ebsp_message("Time runs forward");
    // expect: ($00: Time runs forward)

    t_old = ebsp_host_time();
    backward = 0;
    for (int i = 0; i < 30; i++) {
        bsp_sync();

        // do something because the host is allowed to be slightly slow
        volatile int busyloop = 10000;
        while (busyloop--) {};

        t_new = ebsp_host_time();
        if (t_new <= t_old) {
            backward = 1;
            ebsp_message("Host time does not run forward? %f->%f (high CPU load?)",t_old, t_new);
        }
        t_old = t_new;
    }
    if (backward == 0 && bsp_pid() == 0)
        ebsp_message("Host time runs forward");
    // expect: ($00: Host time runs forward)

    bsp_end();
    return 0;
}
