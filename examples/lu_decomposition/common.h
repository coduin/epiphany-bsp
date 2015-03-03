/*
File: lu_decomposition/common.h

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

#define LOC_M 0x5000
#define LOC_N 0x5004
#define LOC_DIM 0x5008
#define LOC_MATRIX 0x500c

#define LOC_RESULT 0x5700

#define LOC_RS 0x5800
#define LOC_ARK (LOC_RS + sizeof(int) * M)
#define LOC_R (LOC_ARK + sizeof(float) * M)
#define LOC_PI (LOC_R + sizeof(int))
#define LOC_PI_IN (LOC_PI + sizeof(int) * entries_per_col)
#define LOC_ROW_IN (LOC_PI_IN + sizeof(int) * 2)
#define LOC_COL_IN (LOC_ROW_IN + sizeof(float) * dim)
