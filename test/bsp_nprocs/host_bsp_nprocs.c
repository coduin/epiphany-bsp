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

#include <stdio.h>
#include <host_bsp.h>

int main(int argc, char** argv) {
    for (int p = 1; p; ++p)
    {
        bsp_init("e_bsp_nprocs.srec", argc, argv);
        if (p > bsp_nprocs()) {
            bsp_end();
            break;
        }
        bsp_begin(p);
        ebsp_spmd();
        bsp_end();
    }
    // expect: ($00: 1)
    //
    // expect: ($00: 2)
    // expect: ($01: 2)
    //
    // expect: ($00: 3)
    // expect: ($01: 3)
    // expect: ($02: 3)
    //
    // expect: ($00: 4)
    // expect: ($01: 4)
    // expect: ($02: 4)
    // expect: ($03: 4)
    //
    // expect: ($00: 5)
    // expect: ($01: 5)
    // expect: ($02: 5)
    // expect: ($03: 5)
    // expect: ($04: 5)
    //
    // expect: ($00: 6)
    // expect: ($01: 6)
    // expect: ($02: 6)
    // expect: ($03: 6)
    // expect: ($04: 6)
    // expect: ($05: 6)
    //
    // expect: ($00: 7)
    // expect: ($01: 7)
    // expect: ($02: 7)
    // expect: ($03: 7)
    // expect: ($04: 7)
    // expect: ($05: 7)
    // expect: ($06: 7)
    //
    // expect: ($00: 8)
    // expect: ($01: 8)
    // expect: ($02: 8)
    // expect: ($03: 8)
    // expect: ($04: 8)
    // expect: ($05: 8)
    // expect: ($06: 8)
    // expect: ($07: 8)
    //
    // expect: ($00: 9)
    // expect: ($01: 9)
    // expect: ($02: 9)
    // expect: ($03: 9)
    // expect: ($04: 9)
    // expect: ($05: 9)
    // expect: ($06: 9)
    // expect: ($07: 9)
    // expect: ($08: 9)
    //
    // expect: ($00: 10)
    // expect: ($01: 10)
    // expect: ($02: 10)
    // expect: ($03: 10)
    // expect: ($04: 10)
    // expect: ($05: 10)
    // expect: ($06: 10)
    // expect: ($07: 10)
    // expect: ($08: 10)
    // expect: ($09: 10)
    //
    // expect: ($00: 11)
    // expect: ($01: 11)
    // expect: ($02: 11)
    // expect: ($03: 11)
    // expect: ($04: 11)
    // expect: ($05: 11)
    // expect: ($06: 11)
    // expect: ($07: 11)
    // expect: ($08: 11)
    // expect: ($09: 11)
    // expect: ($10: 11)
    //
    // expect: ($00: 12)
    // expect: ($01: 12)
    // expect: ($02: 12)
    // expect: ($03: 12)
    // expect: ($04: 12)
    // expect: ($05: 12)
    // expect: ($06: 12)
    // expect: ($07: 12)
    // expect: ($08: 12)
    // expect: ($09: 12)
    // expect: ($10: 12)
    // expect: ($11: 12)
    //
    // expect: ($00: 13)
    // expect: ($01: 13)
    // expect: ($02: 13)
    // expect: ($03: 13)
    // expect: ($04: 13)
    // expect: ($05: 13)
    // expect: ($06: 13)
    // expect: ($07: 13)
    // expect: ($08: 13)
    // expect: ($09: 13)
    // expect: ($10: 13)
    // expect: ($11: 13)
    // expect: ($12: 13)
    //
    // expect: ($00: 14)
    // expect: ($01: 14)
    // expect: ($02: 14)
    // expect: ($03: 14)
    // expect: ($04: 14)
    // expect: ($05: 14)
    // expect: ($06: 14)
    // expect: ($07: 14)
    // expect: ($08: 14)
    // expect: ($09: 14)
    // expect: ($10: 14)
    // expect: ($11: 14)
    // expect: ($12: 14)
    // expect: ($13: 14)
    //
    // expect: ($00: 15)
    // expect: ($01: 15)
    // expect: ($02: 15)
    // expect: ($03: 15)
    // expect: ($04: 15)
    // expect: ($05: 15)
    // expect: ($06: 15)
    // expect: ($07: 15)
    // expect: ($08: 15)
    // expect: ($09: 15)
    // expect: ($10: 15)
    // expect: ($11: 15)
    // expect: ($12: 15)
    // expect: ($13: 15)
    // expect: ($14: 15)
    //
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
    //
}
