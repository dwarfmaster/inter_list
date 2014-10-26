
#include "fs.h"
#include <stdio.h>
#include <ixp.h>

/* The QID of the first line. */
#define QID_LIST_OFFSET 15
/* The number of QIDs between two lines. */
#define QID_LIST_STEP 5
/* The QID of the text field in a line. */
#define QID_LIST_TEXT 1
/* The QID of the name field in a line. */
#define QID_LIST_NAME 2
/* The QID of the show field in a line. */
#define QID_LIST_SHOW 3

/* The QIDs of the different files (see README for the detail of the files). */
enum { QID_ROOT = 0, /* /               */
    QID_CTL,         /* /ctl            */
    QID_FEED,        /* /feed           */
    QID_BINDINGS,    /* /bindings       */
    QID_TOP,         /* /top            */
    QID_BOT,         /* /bot            */
    QID_COLORS,      /* /colors         */
    QID_LIST,        /* /list           */
    QID_SCROLL,      /* /list/scroll    */
    QID_SELECTION    /* /list/selection */
};

/* The fd of the server. */
static int _fs_fd;
static IxpServer _fs_srv;
static IxpConn   _fs_conn;

/* The filesystem call */
static Ixp9Srv _fs_p9srv;
/* TODO fill _fs_p9srv */

bool fs_init(const char* path)
{
    char buffer[1024];
    _fs_fd = 0;

    snprintf(buffer, 1024, "unix!%s", path);
    _fs_fd = ixp_announce(buffer);
    if(_fs_fd < 0) {
        _fs_fd = 0;
        return false;
    }

    _fs_conn.srv   = &_fs_srv;
    _fs_conn.next  = NULL;
    _fs_conn.fd    = _fs_fd;
    _fs_conn.aux   = &_fs_p9srv;
    _fs_conn.close = NULL;
    _fs_conn.read  = ixp_serve9conn;
    _fs_srv.conn   = &_fs_conn;
    return true;
}

void fs_close()
{
    /* Nothing to do. */
}

int fs_fd()
{
    return _fs_fd;
}

void fs_update()
{
    ixp_serve9conn(&_fs_conn);
}


