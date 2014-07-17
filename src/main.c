
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include "spawn.h"
#include "strformat.h"
#include "curses.h"

int main(int argc, char *argv[])
{
    spawn_t sp;
    char spawnmsg[256];
    char buffer[256];
    const char* lines[2048];
    char* line;
    size_t rd;
    strformat_symbs_t* symbs;
    strformat_t* fmt;
    size_t nb;
    size_t i;
    char c;

    if(argc < 2) {
        printf("Too few arguments.\n");
        return 1;
    }

    /* %s -> the data, %n -> the number of the paquet. */
    symbs = strformat_symbols("sn");
    fmt   = strformat_parse(symbs, argv[1]);
    if(!fmt) {
        printf("Couldn't parse format string.\n");
        return 1;
    }

    sp = spawn_create(argv + 2);
    if(!spawn_ok(sp)) {
        printf("Couldn't spawn argument.\n");
        return 1;
    }

    nb = 0;
    while((rd = spawn_read(sp, spawnmsg, 255)) > 0) {
        spawnmsg[rd] = '\0';

        line = strtok(spawnmsg, "\n");
        while(line) {
            strformat_set(symbs, 's', line);
            sprintf(buffer, "%li", nb + 1);
            strformat_set(symbs, 'n', buffer);

            sprintf(buffer, "%s", strformat_get(fmt));
            line = strdup(buffer);
            if(line) {
                lines[nb] = line;
                ++nb;

                if(nb >= 2048) {
                    printf("Overflow !\n");
                    return 1;
                }
            }

            line = strtok(NULL, "\n");
        }
    }

    if(!curses_init()) {
        printf("Couldn't init curses.\n");
        return 1;
    }

    curses_list_add_lines(nb, lines);

    if(!spawn_ended(sp))
        spawn_wait(sp);
    spawn_close(&sp);

    curses_top_set("Top string, near the sky !!");
    curses_bot_set("Underling.");
    curses_top_colors(COLOR_WHITE, COLOR_BLUE);
    curses_bot_colors(COLOR_BLACK, COLOR_GREEN);
    curses_list_colors(COLOR_CYAN, COLOR_BLACK);
    curses_list_colors_sel(COLOR_RED, COLOR_YELLOW);

    curses_draw();
    while((c = getch()) != 'q') {
        if(c == 'i')
            curses_list_up(1);
        else if(c == 'k')
            curses_list_down(1);
        else if(c == 'l')
            curses_list_right(1);
        else if(c == 'j')
            curses_list_left(1);
        curses_draw();
    }

    strformat_destroy(fmt);
    strformat_symbols_destroy(symbs);

    for(i = 0; i < nb; ++i)
        free((char*)lines[i]);

    curses_end();
    return 0;
}

