/*
File: bsp_host.c

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

#include "host_bsp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <e-loader.h>
#include "common.h"

#define __USE_XOPEN2K
#include <unistd.h>  // readlink, for getting the path to the executable
#define __USE_POSIX199309 1
#include <time.h>


void _microsleep(int microseconds)
{
    struct timespec request, remain;
    request.tv_sec = (int)(microseconds / 1000000);
    request.tv_nsec = (microseconds - 1000000 * request.tv_sec) * 1000;
    if (clock_nanosleep(CLOCK_MONOTONIC, 0, &request, &remain) != 0)
        fprintf(stderr, "ERROR: clock_nanosleep was interrupted.\n");
}

void _get_p_coords(int pid, int* row, int* col)
{
    (*row) = pid / state.cols;
    (*col) = pid % state.cols;
}

// Get the directory that the application is running in
// and store it in state.e_directory
// It will include a trailing slash
void init_application_path()
{
    char path[1024];
    if (readlink("/proc/self/exe", path, 1024) > 0) {
        char * slash = strrchr(path, '/');
        if (slash)
        {
            int count = slash-path + 1;
            memcpy(state.e_directory, path, count);
            state.e_directory[count+1] = 0;
        }
    } else {
        fprintf(stderr, "ERROR: Could not find process directory.\n");
        memcpy(state.e_directory, "./", 3);  // including terminating 0
    }
    return;
}
