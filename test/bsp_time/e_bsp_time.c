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
    float t_old = bsp_time();
            // expect: ($00: Time runs forward)
            // expect: ($00: Time runs forward)
            // expect: ($00: Time runs forward)
    for(int i=0; i<3; i++) {
        bsp_sync();
        float t_new = bsp_time();
        if(bsp_pid() == 0) {
            if(t_new > t_old)
                ebsp_message("Time runs forward", bsp_time());
            else
                ebsp_message("Time runs backwards?", bsp_time());
        }
        t_old = t_new;
    }
    bsp_end();
    return 0;
}
