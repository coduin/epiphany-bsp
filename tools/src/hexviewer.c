#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

typedef enum
{
    KS_DEFAULT = 0
} KEY_STATE;

typedef struct 
{
    int mem_offset;
    int core_shown;
    int nsyncs;
    int num_mod;
    KEY_STATE key_state;
} viewer_state;

viewer_state state;

void read_memory(char* buf, int blocksize)
{
    // fill buffer with nonsense data
    for(int i = 0; i < blocksize; ++i) {
        buf[i] = (char)(rand() % 255);
    }
}

void print_paged_memory(char* buf)
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
            // FIXME output char as hexadecimal properly
            sprintf(val, "%02x ", 
                    (unsigned int)(buf[(state.mem_offset + j) * width + i] + 128));
            printw(val);
        }
        printw(" ");
        for (int i = 0; i < width; ++i) {
            if ((buf[(state.mem_offset + j) * width + i] < 30 ||
                buf[(state.mem_offset + j) * width + i] > 127) || 
                buf[(state.mem_offset + j) * width + i] == '^')
                addch('.');
            else
                addch(buf[(state.mem_offset + j) * width + i]);
        }

        printw("\n");
    }
}

void print_status_bar()
{
    // numbered sync, instructions for going to next sync
    // also which core is being viewed

    int mrow, mcol;
    getmaxyx(stdscr, mrow, mcol);
    move(mrow - 1, 0);

    char status[80];
    sprintf(status, "viewing core: %i, number of syncs: %i", state.core_shown, state.nsyncs);
    printw(status);
}

int handle_input()
{
    // wait for user input
    int ch = getch();

    // switch ch and change paging
    switch (ch)
    {
        case 'q':
            return 0;

        case 'j':
            state.mem_offset++;
            break;

        case 'k':
            if (state.mem_offset > 0)
                state.mem_offset--;
            break;

        case 'g':
            break;

        default:
            break;
    }

    return 1;
}

int main()
{
    srand(1);
    memset(&state, 0, sizeof(viewer_state));

    // initialize curses mode
    initscr();
    // disable line buffering
    raw();
    // disable echoing
    noecho();
    // enable special keys
    keypad(stdscr, TRUE);

    // print string to screen
    attron(A_UNDERLINE);
    printw("epiphany-bsp hexviewer");
    attroff(A_UNDERLINE);

    // refresh the screen (push changes to screen)
    refresh();

    int blocksize = 10000;
    char* buf = malloc(blocksize * sizeof(char));
    read_memory(buf, blocksize);

    while(handle_input())
    {
        print_paged_memory(buf);
        print_status_bar();
        refresh();
    }

    free(buf);



    // close the curses window
    endwin();

    return 0;
}
