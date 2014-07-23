
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
#include "feeder.h"

static int _set_fds(fd_set* fds)
{
    int mfd = 0;
    FD_ZERO(fds);
    FD_SET(0, fds);
    mfd = cmdlifo_fd();
    FD_SET(mfd, fds);
    if(feeder_fd() > 0) {
        mfd = (feeder_fd() > mfd ? feeder_fd() : mfd);
        FD_SET(feeder_fd(), fds);
    }
    return mfd + 1;
}

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

static void _cb_feed(const char* str, void* data) {
    if(data) { } /* avoid warnings. */
    if(!str)
        return;
    feeder_set(str);
}

static void _cb_spawn(const char* str, void* data) {
    if(!data) { } /* avoid warnings */
    if(!str)
        return;
    cmdlifo_push(str);
}

int main(int argc, char *argv[])
{
    bool cont = true;
    char cmd[4096];
    size_t i, size;
    fd_set fds;

    if(argc < 2) {
        printf("Too few arguments.\n");
        return 1;
    }

    cmd[0] = '\0';
    size = 4095;
    for(i = 1; i < (size_t)argc; ++i) {
        strncat(cmd, argv[i], size);
        size = 4095 - strlen(cmd);
        strncat(cmd, " ", size);
        --size;
    }
    fprintf(stderr, "Cmd : %s\n", cmd);

    if(!curses_init()) {
        printf("Couldn't init curses.\n");
        return 1;
    }

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
    cmdparser_add_command("feed",  &_cb_feed,  NULL);
    cmdparser_add_command("spawn", &_cb_spawn, NULL);

    if(!cmdlifo_init()) {
        printf("Coudln't init cmdlifo.\n");
        return 1;
    }
    if(!cmdlifo_push(cmd)) {
        printf("Couldn't push %s to cmdlifo.\n", cmd);
        return 1;
    }

    if(!feeder_init()) {
        printf("Couldn't init feeder.\n");
        return 1;
    }

    if(!events_init()) {
        printf("Couldn't init events.\n");
        return 1;
    }

    curses_draw();
    while(cont) {
        select(_set_fds(&fds), &fds, NULL, NULL, NULL);
        if(FD_ISSET(0, &fds))
            events_process();
        if(FD_ISSET(cmdlifo_fd(), &fds))
            cmdlifo_update();
        if(FD_ISSET(feeder_fd(), &fds))
            feeder_update();
        curses_draw();
    }

    events_quit();
    feeder_quit();
    cmdlifo_quit();
    cmdparser_quit();
    curses_end();
    return 0;
}

