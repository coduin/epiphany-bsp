///////////////////////////////////////////////////////////////////////////////

/*
    File: bsp.h

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

///////////////////////////////////////////////////////////////////////////////

/** Initializes the BSP system. This sets up all the BSP variables and loads
 *  the epiphany BSP program.
 *
 *  @param e_name: A string containing the name of the eBSP program.
 *  @param argc: An integer containing the number of input arguments
 *  @param argv: An array of strings containg the input flags.
 */
void bsp_init(const char* e_name,
        int argc,
        char **argv);

/** Starts the BSP program.
 *
 *  @param nprocs: An integer indicating the number of processors to run on.
 */
void bsp_begin(int nprocs);

/** Finalizes and cleans up the BSP program.
 */
void bsp_end();

/** Returns the number of available processors.
 *
 *  @return nprocs: An integer indicating the number of available processors.
 */
int bsp_nprocs();
