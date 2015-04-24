/*
File: host_e_hello.c

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

#include <stdio.h>
#include <e-hal.h>
#include <e-loader.h>

int main(int argc, char **argv)
{
    e_platform_t platform;
    e_epiphany_t dev;

    e_init(NULL);
    e_reset_system();
    e_get_platform_info(&platform);
    e_open(&dev, 0, 0, 1, 1);
    e_reset_group(&dev);

    e_load_group("bin/e_bsp_memtest.srec", &dev, 0, 0, 1, 1, E_FALSE);

    int test = 12345;
    e_write(&dev, 0, 0, 0x7000, &test, sizeof(int));

    e_start_group(&dev);

    usleep(100000);
    int buf;
    e_read(&dev, 0, 0, 0x7000, &buf, sizeof(int));

    printf("Obtained %i put in %i\n", buf, test); // expect: (Obtained 12346 put in 12345)

    e_finalize();

    printf("Done"); // expect: (Done)
}
