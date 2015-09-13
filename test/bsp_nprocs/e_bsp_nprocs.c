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
    for(int i = 0; i < bsp_nprocs(); ++i) {
        if(i == bsp_pid())
            ebsp_message("%d", bsp_nprocs());
            // expect: ($00: 16)
            // expect: ($01: 16)
            // expect: ($02: 16)
            // expect: ($03: 16)
            // expect: ($04: 16)
            // expect: ($05: 16)
            // expect: ($06: 16)
            // expect: ($07: 16)
            // expect: ($08: 16)
            // expect: ($09: 16)
            // expect: ($10: 16)
            // expect: ($11: 16)
            // expect: ($12: 16)
            // expect: ($13: 16)
            // expect: ($14: 16)
            // expect: ($15: 16)
        bsp_sync();
    }
    bsp_end();
    return 0;
}
