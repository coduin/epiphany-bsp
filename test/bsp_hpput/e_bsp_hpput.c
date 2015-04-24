/*
File: e_bsp_hpput.c

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
    int var = bsp_pid();
    int* unregistered_var=(int*)0x7000;
    char teststr[] = "Default test string!";
    char goodstr[] = "Replacement string.";
    bsp_push_reg(&var, sizeof(int));

    if(bsp_pid() != 2)
        bsp_sync();

    bsp_push_reg(teststr, sizeof(int));

    if(bsp_pid() == 2) //Only core 2 will do both registrations in the same sync
        bsp_sync();     // expect: ($02: BSP ERROR: multiple bsp_push_reg calls within one sync)

    if(bsp_pid() == 1) {
        bsp_hpput(0, &var, &var, 0, sizeof(int));
        bsp_hpput(0, &var, unregistered_var, 0, sizeof(int));//Error
            // expect: ($01: BSP ERROR: could not find bsp var 0x7000)
    }
    if(bsp_pid() == 0) {
        bsp_hpput(1, goodstr, teststr, 0, 19*sizeof(char));
    }

    bsp_sync();
    
    if(bsp_pid() == 0)
        ebsp_message("%d", var);
            // expect: ($00: 1)
    bsp_sync();
    if(bsp_pid() == 1)
        ebsp_message(teststr);
            // expect: ($01: Replacement string.!)

    bsp_end();
    return 0;
}
