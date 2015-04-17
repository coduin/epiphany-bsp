/*
File: sizeof_check.cpp

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

// When compiling on x86_64 we want to check for any alignment issues
// between epiphany and ARM compiles. Since we can not run the epiphany
// compiled code, and we do not want to look into the dissasembly every time,
// a faster way is required.
// Using the C++ compiler, there is a trick:
//  http://stackoverflow.com/questions/2008398/is-it-possible-to-print-out-the-size-of-a-c-class-at-compile-time
//  http://stackoverflow.com/questions/7931358/printing-sizeoft-at-compile-time

#include "common.h"

template<class T, int N = sizeof(T)> 
struct size_as_warning
{ 
    char operator()() { return N + 256; } //deliberately causing overflow
};

int main()
{
    size_as_warning<ebsp_data_request>()();
    size_as_warning<ebsp_payload_buffer>()();
    size_as_warning<ebsp_message_header>()();
    size_as_warning<ebsp_message_queue>()();
    size_as_warning<ebsp_comm_buf>()();
    return 0;
}

