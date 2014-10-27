
#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include "spawn.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include "events.h"
#include "cmdlifo.h"
#include "feeder.h"
#include "commands.h"
#include "bars.h"
#include "fs.h"

static void _cb_common()
{
    bars_update();
    curses_draw();
}

static void _cb_events(IxpConn* conn)
{
    if(conn) {} /* Avoid warnings. */
    events_process();
    _cb_common();
}

static void _cb_feeder(IxpConn* conn)
{
    if(conn) {} /* Avoid warnings. */
    feeder_update();
    _cb_common();
}

static void _cb_cmdlifo(IxpConn* conn)
{
    int fd = conn->fd;
    int feed = feeder_fd();
    IxpServer* srv = conn->srv;

    cmdlifo_update();
    if(fd != cmdlifo_fd()) {
        ixp_hangup(conn);
        ixp_listen(srv, cmdlifo_fd(), NULL, _cb_cmdlifo, NULL);
    }

    for(conn = srv->conn; conn; conn = conn->next) {
        if(conn->fd == feed) {
            if(feed != feeder_fd()) {
                ixp_hangup(conn);
                ixp_listen(srv, feeder_fd(), NULL, _cb_feeder, NULL);
            }
            break;
        }
    }
    _cb_common();
}

int main(int argc, char *argv[])
{
    char cmd[4096];
    size_t i, size;
    IxpServer srv;

    if(argc < 3) {
        printf("Too few arguments.\n");
        return 1;
    }
    setlocale(LC_ALL, "");

    cmd[0] = '\0';
    size = 4095;
    for(i = 2; i < (size_t)argc; ++i) {
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
    commands_setup(&srv.running);

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

    if(!fs_init(argv[1])) {
        printf("Couldn't init the fs server on %s\n", argv[1]);
        return 1;
    }

    /* Setting up the ixp server. */
    srv.conn    = NULL;
    srv.running = true;
    ixp_listen(&srv, feeder_fd(),  NULL,     _cb_feeder,  NULL);
    ixp_listen(&srv, cmdlifo_fd(), NULL,     _cb_cmdlifo, NULL);
    ixp_listen(&srv, 0,            NULL,     _cb_events,  NULL);
    ixp_listen(&srv, fs_fd(),      fs_aux(), fs_update,   NULL);

    curses_draw();
    while(srv.running)
        ixp_serverloop(&srv);

    fs_close();
    events_quit();
    feeder_quit();
    cmdlifo_quit();
    cmdparser_quit();
    bars_quit();
    curses_end();
    return 0;
}

