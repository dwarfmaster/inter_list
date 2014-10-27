
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

/* Macros to get the QID of the field of a line. */
#define GET_TEXT_QID(n) (QID_LIST_OFFSET + QID_LIST_STEP * (n) + QID_LIST_TEXT)
#define GET_NAME_QID(n) (QID_LIST_OFFSET + QID_LIST_STEP * (n) + QID_LIST_NAME)
#define GET_SHOW_QID(n) (QID_LIST_OFFSET + QID_LIST_STEP * (n) + QID_LIST_SHOW)
/* Get the line nb from a QID */
#define GET_LINE_ID(q) (((q) - QID_LIST_OFFSET) / QID_LIST_STEP)
/* Get the id of the field in a line from a QID. */
#define GET_FIELD_ID(q) (((q) - QID_LIST_OFFSET) % QID_LIST_STEP)

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
    QID_SELECTION,   /* /list/selection */
    QID_END          /* The last of specials QIDs */
};

/* The fd of the server. */
static int _fs_fd;

/* The filesystem call */
static void _fs_attach(Ixp9Req* r);
static void _fs_open(Ixp9Req* r);
static void _fs_clunk(Ixp9Req* r);
static void _fs_walk(Ixp9Req* r);
static void _fs_stat(Ixp9Req* r);
static void _fs_read(Ixp9Req* r);
static void _fs_write(Ixp9Req* r);
static void _fs_wstat(Ixp9Req* r);

static Ixp9Srv _fs_p9srv = {
    .open   = _fs_open,
    .clunk  = _fs_clunk,
    .walk   = _fs_walk,
    .read   = _fs_read,
    .stat   = _fs_stat,
    .write  = _fs_write,
    .wstat  = _fs_wstat,
    .attach = _fs_attach
};

static void _fs_attach(Ixp9Req* r)
{
    r->fid->qid.type = P9_QTDIR;
    r->fid->qid.path = QID_ROOT;
    r->ofcall.rattach.qid = r->fid->qid;
    ixp_respond(r, NULL);
}

static void _fs_open(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_clunk(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_walk(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_stat(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_read(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_write(Ixp9Req* r)
{
    /* TODO */
}

static void _fs_wstat(Ixp9Req* r)
{
    /* Pretend it worked. */
    ixp_respond(r, NULL);
}

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
    /* TODO */
}


