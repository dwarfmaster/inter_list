
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
#include "commands.h"
#include "bars.h"

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

    if(!curses_init()) {
        printf("Couldn't init curses.\n");
        return 1;
    }

    if(!bars_init()) {
        printf("Couldn't init bars.\n");
        return 1;
    }
    bars_top_set(NULL);
    bars_bot_set(NULL);

    if(!cmdparser_init()) {
        printf("Couldn't init cmdparse.\n");
        return 1;
    }
    commands_setup(&cont);

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
        bars_update();
        curses_draw();
    }

    events_quit();
    feeder_quit();
    cmdlifo_quit();
    cmdparser_quit();
    bars_quit();
    curses_end();
    return 0;
}

