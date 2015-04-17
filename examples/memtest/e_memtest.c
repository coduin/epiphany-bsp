/*
File: e_memtest.c

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

    int p = bsp_pid();

    char a = 0;
    char b = 0;
    char c = 0;
    bsp_push_reg(&a, sizeof(char));
    bsp_sync();
    bsp_push_reg(&b, sizeof(char));
    bsp_sync();

    if (p == 0)
    {
        c = 'y';
        bsp_hpput(3, &c, &a, 0, sizeof(char));
        bsp_hpput(3, &c, &b, 0, sizeof(char));
    }

    bsp_end();

    return 0;
}
