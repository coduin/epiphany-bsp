// # Keys:
// [x] num/mod + g, G
// [ ] 'n' = 'next sync"
// [x] 'l', 'h' = next/prev core
//
// # Features
// [ ] Red for memory that changed from last run
// [ ] actually dump memory from epiphany.
// [ ] Optional: notes on memory regions
// [ ] Fix maximum cores/memory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <e-hal.h>
#include <host_bsp.h>

typedef enum
{
    KS_DEFAULT = 0,
    KS_G_PRESSED,
    KS_NUM_MOD
} KEY_STATE;

typedef struct 
{
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
} e_h_viewer_state;

e_h_viewer_state state;

void _e_h_read_memory(unsigned char* buf, int blocksize)
{
    co_read(state.core_shown, (void*)0x0000, buf, blocksize);
}

void _e_h_print_paged_memory(unsigned char* buf)
{
    int width = 16;

    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);

    move(2, 0);
    // need offset
    for (int j = 0; j < mrow - 4; ++j) {
        char loc[8];
        sprintf(loc, "0x%04x   ",  (state.mem_offset + j) * width);
        printw(loc);
        for (int i = 0; i < width; ++i) {
            char val[4];
            sprintf(val, "%02x ", 
                    buf[(state.mem_offset + j) * width + i]);
            printw(val);
        }
        printw("  ");
        for (int i = 0; i < width; ++i) {
            if ((buf[(state.mem_offset + j) * width + i] < '#' ||
                buf[(state.mem_offset + j) * width + i] > '}'))
                addch('.');
            else
                addch(buf[(state.mem_offset + j) * width + i]);
        }

        printw("\n");
    }
}

void _e_h_print_status_bar()
{
    // numbered sync, instructions for going to next sync
    // also which core is being viewed

    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    move(mrow - 1, 0);
    printw("\n");

    char status[80];
    sprintf(status, "viewing core: %i, number of syncs: %i",
            state.core_shown,
            state.nsyncs);
    printw(status);
}

int _e_h_max_offset()
{
    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    return state.mem_max_offset - (mrow - 4);
}

int _e_h_handle_input()
{
    // wait for user input
    int ch = getch();
    int digit = 0;
    int maxoff = _e_h_max_offset();

    // switch ch and change paging
    switch (ch)
    {
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
            state.num_mod *= 10;
            state.num_mod += digit;
            break;

        case 'q':
            return 0;

        case 'j':

            if(state.num_mod > 0) {
                state.mem_offset += state.num_mod;
                state.num_mod = 0;
            } 
            else {
                state.mem_offset++;
            }

            if(state.mem_offset > maxoff)
                state.mem_offset = maxoff;

            break;

        case 'l':
            state.core_shown++;
            if(state.core_shown > state.core_max)
                state.core_shown = state.core_max;
            break;

        case 'h':
            if (state.core_shown > 0)
                state.core_shown--;
            break;

        case 'k':
            if(state.num_mod > 0) {
                state.mem_offset -= state.num_mod;
                if(state.mem_offset < 0)
                    state.mem_offset = 0;
                state.num_mod = 0;
            }
            else if (state.mem_offset > 0)
                state.mem_offset--;
            break;

        case 'g':
            if(state.num_mod > 0) {
                state.mem_offset = state.num_mod;
                state.num_mod = 0;
            } else {
                if(state.key_state == KS_G_PRESSED) {
                    state.mem_offset = 0;
                    state.key_state = KS_DEFAULT;
                } else {
                    state.key_state = KS_G_PRESSED;
                }
            }
            break;

        case 'G':
            state.mem_offset = maxoff;
            break;

        case 'n':
            // FIXME: return control to bsp lib
            state.nsyncs++;
            break;

        default:
            break;
    }

    return 1;
}

void ebsp_hexviewer_init()
{
    memset(&state, 0, sizeof(e_h_viewer_state));

    state.mem_max = 0x8000;
    state.mem_max_offset = state.mem_max / 0x10;
    state.core_max = bsp_nprocs();

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

void ebsp_hexviewer_run()
{
    // print string to screen
    attron(A_UNDERLINE);
    printw("epiphany-bsp hexviewer");
    attroff(A_UNDERLINE);

    // refresh the screen (push changes to screen)
    refresh();

    int blocksize = state.mem_max;
    unsigned char* buf = malloc(blocksize * sizeof(unsigned char));
    _e_h_read_memory(buf, blocksize);

    do
    {
        if (state.data_dirty) {
            _e_h_read_memory(buf, blocksize);
        }
        _e_h_print_paged_memory(buf);
        _e_h_print_status_bar();

        // FIXME: return control to BSP when next sync
        // ...

        refresh();
    } while (_e_h_handle_input());

    free(buf);

    // close the curses window
    endwin();
}

int main()
{
    ebsp_hexviewer_init();
    ebsp_hexviewer_run();

    return 0;
}
