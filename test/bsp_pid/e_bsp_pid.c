/*
File: e_bsp_pid.c

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
    for(int i=0; i<bsp_nprocs(); i++) {
        if(i == bsp_pid())
            ebsp_message("%d", bsp_pid());
            // expect: ($00: 0)
            // expect: ($01: 1)
            // expect: ($02: 2)
            // expect: ($03: 3)
            // expect: ($04: 4)
            // expect: ($05: 5)
            // expect: ($06: 6)
            // expect: ($07: 7)
            // expect: ($08: 8)
            // expect: ($09: 9)
            // expect: ($10: 10)
            // expect: ($11: 11)
            // expect: ($12: 12)
            // expect: ($13: 13)
            // expect: ($14: 14)
            // expect: ($15: 15)
        bsp_sync();
    }
    bsp_end();
    return 0;
}
