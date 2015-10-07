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
#include <stdlib.h>
#include <string.h>

#include <e-hal.h>
#include <host_bsp.h>

#include <ncurses.h>
#include <signal.h>

typedef enum { KS_DEFAULT = 0, KS_G_PRESSED, KS_NUM_MOD } KEY_STATE;

typedef struct {
    // maximum value for memory and cores
    int mem_max;
    int mem_max_offset;
    int core_max;

    // set to 1 if required to repull data from eCore
    int data_dirty;

    // current viewing position and information
    int mem_offset;
    int core_shown;
    int nsyncs;
    int num_mod;
    KEY_STATE key_state;

    // buffer for memory
    unsigned char* buf;
} e_h_viewer_state;

e_h_viewer_state i_state;

void _e_h_read_memory(unsigned char* buf, int blocksize) {
    ebsp_read(i_state.core_shown, (off_t)0x0000, buf, blocksize);
}

void _e_h_print_paged_memory(unsigned char* buf) {
    int width = 16;

    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    (void)mcol; // prevents mcol unused warning

    move(2, 0);
    // need offset
    for (int j = 0; j < mrow - 4; ++j) {
        char loc[10];
        snprintf(loc, sizeof(loc), "0x%04x   ",
                 (i_state.mem_offset + j) * width);
        printw(loc);
        for (int i = 0; i < width; ++i) {
            char val[6];
            snprintf(val, sizeof(loc), "%02x ",
                     buf[(i_state.mem_offset + j) * width + i]);
            printw(val);
        }
        printw("  ");
        for (int i = 0; i < width; ++i) {
            if ((buf[(i_state.mem_offset + j) * width + i] < '#' ||
                 buf[(i_state.mem_offset + j) * width + i] > '}'))
                addch('.');
            else
                addch(buf[(i_state.mem_offset + j) * width + i]);
        }

        printw("\n");
    }
}

void _e_h_print_status_bar() {
    // numbered sync, instructions for going to next sync
    // also which core is being viewed

    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    (void)mcol; // prevents mcol unused warning
    move(mrow - 1, 0);
    printw("\n");

    char status[80];
    snprintf(status, sizeof(status), "viewing core: %i, number of syncs: %i",
             i_state.core_shown, i_state.nsyncs);
    printw(status);
}

int _e_h_max_offset() {
    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    (void)mcol; // prevents mcol unused warning
    return i_state.mem_max_offset - (mrow - 4);
}

int _e_h_handle_input() {
    // wait for user input
    int ch = getch();
    int digit = 0;
    int maxoff = _e_h_max_offset();

    // switch ch and change paging
    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        digit = ch - '0';
        i_state.num_mod *= 10;
        i_state.num_mod += digit;
        break;

    case 'q':
        return 0;

    case 'j':

        if (i_state.num_mod > 0) {
            i_state.mem_offset += i_state.num_mod;
            i_state.num_mod = 0;
        } else {
            i_state.mem_offset++;
        }

        if (i_state.mem_offset > maxoff)
            i_state.mem_offset = maxoff;

        break;

    case 'l':
        if (i_state.core_shown < i_state.core_max) {
            i_state.core_shown++;
            i_state.data_dirty = 1;
        }
        break;

    case 'h':
        if (i_state.core_shown > 0) {
            i_state.core_shown--;
            i_state.data_dirty = 1;
        }
        break;

    case 'k':
        if (i_state.num_mod > 0) {
            i_state.mem_offset -= i_state.num_mod;
            if (i_state.mem_offset < 0)
                i_state.mem_offset = 0;
            i_state.num_mod = 0;
        } else if (i_state.mem_offset > 0) {
            i_state.mem_offset--;
        }
        break;

    case 'g':
        if (i_state.num_mod > 0) {
            int hex = 0;
            int cur_pow = 1;
            while (i_state.num_mod > 0) {
                hex += (i_state.num_mod % 10) * cur_pow;
                i_state.num_mod /= 10;
                cur_pow *= 16;
            }
            i_state.mem_offset = hex / 0x10;
            if (i_state.mem_offset > maxoff) {
                i_state.mem_offset = maxoff;
            }
        } else {
            if (i_state.key_state == KS_G_PRESSED) {
                i_state.mem_offset = 0;
                i_state.key_state = KS_DEFAULT;
            } else {
                i_state.key_state = KS_G_PRESSED;
            }
        }
        break;

    case 'G':
        i_state.mem_offset = maxoff;
        break;

    case 'n':
        i_state.nsyncs++;
        return 0;

    default:
        break;
    }

    return 1;
}

void ebsp_inspector_update() {
    // print string to screen
    attron(A_UNDERLINE);
    printw("epiphany-bsp inspector");
    attroff(A_UNDERLINE);

    // refresh the screen (push changes to screen)
    refresh();

    int blocksize = i_state.mem_max;
    _e_h_read_memory(i_state.buf, blocksize);

    do {
        if (i_state.data_dirty) {
            _e_h_read_memory(i_state.buf, blocksize);
        }
        _e_h_print_paged_memory(i_state.buf);
        _e_h_print_status_bar();

        refresh();

        // FIXME: differentiate between different return codes,
        // or add additional callback for finalize
    } while (_e_h_handle_input());
}

void ebsp_inspector_finalize() {
    ebsp_inspector_update();

    // free memory buffer
    free(i_state.buf);

    // close the curses window
    endwin();
}

void ebsp_inspector_finish() {
    bsp_end();

    // free memory buffer
    free(i_state.buf);

    // close the curses window
    endwin();

    // exit
    exit(0);
}

void ebsp_inspector_enable() {
    // TODO(JW)
    // terminate with signals properly
    signal(SIGINT, ebsp_inspector_finish);

    // TODO(JW)
    // redirect stdout
    // ... (freopen, dup)

    memset(&i_state, 0, sizeof(e_h_viewer_state));

    i_state.mem_max = 0x8000;
    i_state.mem_max_offset = i_state.mem_max / 0x10;
    i_state.core_max = bsp_nprocs() - 1;

    int blocksize = i_state.mem_max;
    i_state.buf = malloc(blocksize * sizeof(unsigned char) * 2);

    // register with bsp system
    ebsp_set_sync_callback(&ebsp_inspector_update);
    ebsp_set_end_callback(&ebsp_inspector_finalize);

    // initialize curses mode
    initscr();

    // disable line buffering
    raw();

    // disable echoing
    noecho();

    // enable special keys
    keypad(stdscr, TRUE);

    // disable cursor
    curs_set(0);
}
