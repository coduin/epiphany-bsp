/*
File: e_bsp_raw_time.s

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

//unsigned int bsp_raw_time()
//{
//    unsigned int raw_time = e_ctimer_get(E_CTIMER_0);
//    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
//    return E_CTIMER_MAX - raw_time;
//}

.file    "e_bsp_raw_time.s";

.section .text;
.type    _bsp_raw_time, %function;
.global  _bsp_raw_time;

.balign 4;
_bsp_raw_time:
    // Set r1 to MAX_UINT = -1
    mov r1, %low(#-1);
    movt r1, %high(#-1);
    // Read the timer into r0
    // Write MAX_UINT to the timer
    movfs r0, ctimer0;
    movts ctimer0, r1;
    // r0 = MAX_UINT - r0
    // Implemented by an xor
    eor r0, r0, r1;
    // Return
    rts;

.size    _bsp_raw_time, .-_bsp_raw_time;
