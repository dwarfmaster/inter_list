
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/select.h>
#include "spawn.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include "events.h"
#include "cmdlifo.h"

static void _cb_up(const char* str, void* data) {
    unsigned int up = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &up);
    curses_list_up(up);
}

static void _cb_down(const char* str, void* data) {
    unsigned int down = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &down);
    curses_list_down(down);
}

static void _cb_left(const char* str, void* data) {
    unsigned int left = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &left);
    curses_list_left(left);
}

static void _cb_right(const char* str, void* data) {
    unsigned int right = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &right);
    curses_list_right(right);
}

static void _cb_quit(const char* str, void* data) {
    if(str) { } /* avoid warnings */
    bool* cont = data;
    *cont = false;
}

static void _cb_exe(const char* str, void* data) {
    if(data) { } /* avoid warnings */
    if(str)
        cmdparser_parse(str);
}

static void _cb_map(const char* str, void* data) {
    if(data) { } /* avoid warnings */
    if(!str)
        return;

    char* strtokbuf;
    char* keys;
    char* action;
    char* used = strdup(str);

    keys = strtok_r(used, " ", &strtokbuf);
    if(keys) {
        action = strtok_r(NULL, "", &strtokbuf);
        if(action)
            events_add(keys, action);
    }
    free(used);
}

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
    bool cont = true;
    fd_set fds;

    if(argc < 3) {
        printf("Too few arguments.\n");
        return 1;
    }

    /* %s -> the data, %n -> the number of the paquet. */
    symbs = strformat_symbols("sn");
    fmt   = strformat_parse(symbs, argv[2]);
    if(!fmt) {
        printf("Couldn't parse format string.\n");
        return 1;
    }

    sp = spawn_create(argv + 3);
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

    if(!spawn_ended(sp))
        spawn_wait(sp);
    spawn_close(&sp);

    if(!curses_init()) {
        printf("Couldn't init curses.\n");
        return 1;
    }
    curses_list_add_lines(nb, lines);

    curses_top_set("Top string, near the sky !!");
    curses_bot_set("Underling.");
    curses_top_colors(COLOR_WHITE, COLOR_BLUE);
    curses_bot_colors(COLOR_BLACK, COLOR_GREEN);
    curses_list_colors(COLOR_CYAN, COLOR_BLACK);
    curses_list_colors_sel(COLOR_RED, COLOR_YELLOW);

    if(!cmdparser_init()) {
        printf("Couldn't init cmdparse.\n");
        return 1;
    }

    cmdparser_add_command("up",    &_cb_up,    NULL);
    cmdparser_add_command("down",  &_cb_down,  NULL);
    cmdparser_add_command("right", &_cb_right, NULL);
    cmdparser_add_command("left",  &_cb_left,  NULL);
    cmdparser_add_command("quit",  &_cb_quit,  &cont);
    cmdparser_add_command("exe",   &_cb_exe,   NULL);
    cmdparser_add_command("map",   &_cb_map,   NULL);

    if(!cmdlifo_init()) {
        printf("Coudln't init cmdlifo.\n");
        return 1;
    }
    if(!cmdlifo_push(argv[1])) {
        printf("Couldn't push %s to cmdlifo.\n", argv[1]);
        return 1;
    }

    if(!events_init()) {
        printf("Couldn't init events.\n");
        return 1;
    }

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(cmdlifo_fd(), &fds);

    curses_draw();
    while(cont) {
        select(cmdlifo_fd() + 1, &fds, NULL, NULL, NULL);
        if(FD_ISSET(0, &fds))
            events_process();
        if(FD_ISSET(cmdlifo_fd(), &fds))
            cmdlifo_update();

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(cmdlifo_fd(), &fds);
        curses_draw();
    }

    strformat_destroy(fmt);
    strformat_symbols_destroy(symbs);

    for(i = 0; i < nb; ++i)
        free((char*)lines[i]);

    events_quit();
    cmdlifo_quit();
    cmdparser_quit();
    curses_end();
    return 0;
}

