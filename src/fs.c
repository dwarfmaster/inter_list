
#include "fs.h"
#include "feeder.h"
#include "bars.h"
#include "curses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The QID of the first line. */
#define QID_LIST_OFFSET 15
/* The number of QIDs between two lines. */
#define QID_LIST_STEP 5
/* The QID of the line dir field in a line. */
#define QID_LIST_LINE 0
/* The QID of the text field in a line. */
#define QID_LIST_TEXT 1
/* The QID of the name field in a line. */
#define QID_LIST_NAME 2
/* The QID of the show field in a line. */
#define QID_LIST_SHOW 3

/* Macros to get the QID of the field of a line. */
#define GET_LINE_QID(n) (QID_LIST_OFFSET + QID_LIST_STEP * (n) + QID_LIST_LINE)
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
    uint32_t qid;
    const char* str;
    size_t i;
    char buf[64];
    feeder_iterator_t it;

    qid = r->fid->qid.path;
    r->fid->aux = NULL;

    /* TODO : most of this is only needed for read opens. */
    if(qid < QID_END) {
        switch(qid) {
            case QID_FEED:
                str = feeder_get();
                if(str) r->fid->aux = strdup(str);
                else    r->fid->aux = strdup("");
                break;
            case QID_TOP:
                str = bars_top_get();
                if(str) r->fid->aux = strdup(str);
                else    r->fid->aux = strdup("");
                break;
            case QID_BOT:
                str = bars_bot_get();
                if(str) r->fid->aux = strdup(str);
                else    r->fid->aux = strdup("");
                break;
            case QID_SCROLL:
                if(curses_list_get_mode())
                    r->fid->aux = strdup("pager");
                else
                    r->fid->aux = strdup("list");
                break;
            case QID_SELECTION:
                i = curses_list_get();
                snprintf(buf, 64, "%lu", i);
                r->fid->aux = strdup(buf);
                break;
            case QID_LIST:
                r->fid->aux = malloc(sizeof(feeder_iterator_t));
                if(!r->fid->aux) {
                    ixp_respond(r, "out of memory");
                    return;
                }
                it = feeder_begin();
                memcpy(r->fid->aux, &it, sizeof(feeder_iterator_t));
                break;
            case QID_BINDINGS:
                /* TODO */
            case QID_CTL:
            case QID_COLORS:
                /* Nothing to do. */
                break;
            default:
                goto open_error;
        }
    }

    else if(GET_LINE_ID(qid) < feeder_end().id) {
        i = GET_LINE_ID(qid);
        it = feeder_begin();
        feeder_next_real(&it, i);
        switch(GET_FIELD_ID(qid)) {
            case QID_LIST_TEXT:
                r->fid->aux = strdup(feeder_get_it_text(it));
                break;
            case QID_LIST_NAME:
                r->fid->aux = strdup(feeder_get_it_name(it));
                break;
            case QID_LIST_SHOW:
                if(feeder_is_it_hidden(it))
                    r->fid->aux = strdup("0");
                else
                    r->fid->aux = strdup("1");
                break;
            case QID_LIST_LINE:
                /* Nothing to do. */
                break;
            default:
                goto open_error;
        }
    }

    else {
open_error:
        ixp_respond(r, "invalid qid in open");
        return;
    }
    ixp_respond(r, NULL);
}

static void _fs_clunk(Ixp9Req* r)
{
    void* aux = r->fid->aux;
    if(aux)
        free(aux);
    ixp_respond(r, NULL);
}

size_t _fs_pow(size_t nb, size_t pow)
{
    size_t i;
    size_t ret = 1;

    for(i = 0; i < pow; ++i)
        ret *= nb;
    return ret;
}

bool _fs_strtol(const char* str, size_t* l)
{
    size_t len = strlen(str);
    size_t ret, i;

    ret = 0;
    for(i = 0; i < len; ++i) {
        if(str[i] < '0' || str[i] > '9')
            return false;
        ret += (str[i] - '0') * _fs_pow(10, len - i - 1);
    }

    if(l)
        *l = ret;
    return true;
}

static void _fs_walk(Ixp9Req* r)
{
    char buf[256];
    uint16_t cwd, qid, path;
    uint8_t type;
    int i;
    size_t id;
    const char* pth;

    cwd = r->fid->qid.path;
    r->ofcall.rwalk.nwqid = 0;
    for(i = 0; i < r->ifcall.twalk.nwname; ++i) {
        pth = r->ifcall.twalk.wname[i];
        type = P9_QTFILE;

        /* Contents of the root directory. */
        if(cwd == 0) {
            if(strcmp(pth, "ctl") == 0)           path = QID_CTL;
            else if(strcmp(pth, "feed") == 0)     path = QID_FEED;
            else if(strcmp(pth, "bindings") == 0) path = QID_BINDINGS;
            else if(strcmp(pth, "top") == 0)      path = QID_TOP;
            else if(strcmp(pth, "bot") == 0)      path = QID_BOT;
            else if(strcmp(pth, "colors") == 0)   path = QID_COLORS;
            else if(strcmp(pth, "list") == 0) {
                path = QID_LIST;
                type = P9_QTDIR;
            }
            else goto walk_error;
        }

        /* Contents of the /list directory */
        else if(cwd == QID_LIST_LINE) {
            if(strcmp(pth, "scroll") == 0)         path = QID_SCROLL;
            else if(strcmp(pth, "selection") == 0) path = QID_SELECTION;
            else if(_fs_strtol(pth, &id) && id < feeder_end().id) {
                type = P9_QTDIR;
                path = GET_LINE_QID(id);
            }
            else goto walk_error;
        }

        /* Contents of any /list/nb directory */
        else if(GET_FIELD_ID(cwd) == QID_LIST_LINE
                && (id = GET_LINE_ID(cwd)) < feeder_end().id)  {
            if(strcmp(pth, "text") == 0)
                path = GET_TEXT_QID(id);
            else if(strcmp(pth, "name") == 0)
                path = GET_NAME_QID(id);
            else if(strcmp(pth, "show") == 0)
                path = GET_SHOW_QID(id);
            else goto walk_error;
        }

        /* Error */
        else {
walk_error:
            snprintf(buf, sizeof(buf),
                    "%s: no such file or directory", pth);
            ixp_respond(r, buf);
            return;
        }

        qid = r->ofcall.rwalk.nwqid;
        r->ofcall.rwalk.wqid[qid].type    = type;
        r->ofcall.rwalk.wqid[qid].path    = path;
        r->ofcall.rwalk.wqid[qid].version = 0;
        ++r->ofcall.rwalk.nwqid;
    }
    ixp_respond(r, NULL);
}

static void _fs_dostat(IxpStat* st, uint32_t path)
{
    if(path <= QID_END) {
        st->type     = P9_QTFILE;
        st->qid.type = P9_QTFILE;
        st->qid.path = path;
        st->mode     = 0600;
        st->length   = 0;
        switch(path) {
            case QID_ROOT:
                st->type     = P9_QTDIR;
                st->qid.type = P9_QTDIR;
                st->mode     = 0500 | P9_DMDIR;
                st->name     = "";
                break;
            case QID_CTL:
                st->mode   = 0200;
                st->name   = "ctl";
                break;
            case QID_FEED:
                st->name   = "feed";
                break;
            case QID_BINDINGS:
                st->name   = "bindings";
                break;
            case QID_TOP:
                st->name   = "top";
                break;
            case QID_BOT:
                st->name   = "bot";
                break;
            case QID_COLORS:
                st->name   = "colors";
                st->length = 8; /* The number of color pairs. */
                break;
            case QID_LIST:
                st->type     = P9_QTDIR;
                st->qid.type = P9_QTDIR;
                st->mode     = 0500 | P9_DMDIR;
                st->name     = "list";
                st->length   = 0; /* TODO get the number of lines. */
                break;
            case QID_SCROLL:
                st->name = "scroll";
                break;
            case QID_SELECTION:
                st->name = "selection";
                break;
            default:
                /* Shouldn't happen. */
                break;
        }
    }
    else {
        size_t id = GET_LINE_ID(path);
        char buffer[64];

        st->type     = P9_QTFILE;
        st->qid.type = P9_QTFILE;
        st->qid.path = path;
        st->mode     = 0400;
        st->length   = 0;
        switch(GET_FIELD_ID(path)) {
            case QID_LIST_LINE:
                snprintf(buffer, 64, "%lu", id);
                st->type     = P9_QTDIR;
                st->qid.type = P9_QTDIR;
                st->mode     = 0500 | P9_DMDIR;
                /* TODO may break : buffer not available when leaving. */
                st->name     = buffer;
                break;
            case QID_LIST_TEXT:
                st->name = "text";
                break;
            case QID_LIST_NAME:
                st->name = "name";
                break;
            case QID_LIST_SHOW:
                st->mode = 0600;
                st->name = "show";
                break;
            default:
                /* Shouldn't happen. */
                break;
        }
    }
    st->uid = st->gid = st->muid = "";
}

static void _fs_stat(Ixp9Req* r)
{
    IxpStat st = { 0 };
    IxpMsg m;
    char buf[512];

    m = ixp_message(buf, sizeof(buf), MsgPack);
    _fs_dostat(&st, r->fid->qid.path);
    ixp_pstat(&m, &st);
    r->ofcall.rstat.nstat = ixp_sizeof_stat(&st);
    if(!(r->ofcall.rstat.stat = malloc(r->ofcall.rstat.nstat))) {
        r->ofcall.rstat.nstat = 0;
        ixp_respond(r, "out of memory");
        return;
    }
    memcpy(r->ofcall.rstat.stat, m.data, r->ofcall.rstat.nstat);
    ixp_respond(r, NULL);
}

#define DOSTAT(x) _fs_dostat(&st, (x)); \
                  ixp_pstat(&m, &st); \
                  r->ofcall.rread.count += ixp_sizeof_stat(&st);

static void _fs_read(Ixp9Req* r)
{
    uint32_t path = r->fid->qid.path;
    void* aux = r->fid->aux;
    r->ofcall.rread.count = 0;

    if(path == QID_ROOT
            || path == QID_LIST
            || GET_FIELD_ID(path) == QID_LIST_LINE) {
        IxpStat st = { 0 };
        IxpMsg m;
        char buf[512];
        m = ixp_message(buf, sizeof(buf), MsgPack);
        size_t i;
        feeder_iterator_t it;

        switch(path) {
            case QID_ROOT:
                if(r->ifcall.tread.offset > 0) {
                    /* Hack : consider the directory can be read in one go. */
                    ixp_respond(r, NULL);
                    return;
                }
                for(i = 1; i <= QID_LIST; ++i)
                    DOSTAT(i);
                break;

            case QID_LIST:
                it = *(feeder_iterator_t*)aux;
                if(!it.valid) {
                    ixp_respond(r, NULL);
                    return;
                } else if(r->ifcall.tread.offset == 0) {
                    DOSTAT(QID_SCROLL);
                    DOSTAT(QID_SELECTION);
                }
                _fs_dostat(&st, it.id);
                i = ixp_sizeof_stat(&st);
                while(it.valid
                        && r->ofcall.rread.count + i < r->ifcall.tread.count) {
                    ixp_pstat(&m, &st);
                    r->ofcall.rread.count += i;
                    feeder_next_real(&it, 1);
                    if(!it.valid)
                        break;
                    _fs_dostat(&st, it.id);
                    i = ixp_sizeof_stat(&st);
                }
                *(feeder_iterator_t*)r->fid->aux = it;
                break;

            default:
                i = GET_LINE_ID(path);
                if(r->ifcall.tread.offset > 0) {
                    /* Hack : consider the directory can be read in one go. */
                    ixp_respond(r, NULL);
                    return;
                }
                DOSTAT(GET_TEXT_QID(i));
                DOSTAT(GET_NAME_QID(i));
                DOSTAT(GET_SHOW_QID(i));
                break;
        }

        if(!(r->ofcall.rread.data = malloc(r->ofcall.rread.count))) {
            r->ofcall.rread.count = 0;
            ixp_respond(r, "out of memory");
            return;
        }
        memcpy(r->ofcall.rread.data, m.data, r->ofcall.rread.count);
    }

    /* Content of a simple file. */
    else if(aux) {
        char* buf = aux;
        size_t len = strlen(buf);
        if(r->ifcall.tread.offset < len) {
            if(r->ifcall.tread.offset + r->ifcall.tread.count > len)
                r->ofcall.rread.count = len - r->ifcall.tread.offset;
            else
                r->ofcall.rread.count = r->ifcall.tread.count;
            if(!(r->ofcall.rread.data = malloc(r->ofcall.rread.count))) {
                r->ofcall.rread.count = 0;
                ixp_respond(r, "out of memory");
                return;
            }
            memcpy(r->ofcall.rread.data, buf + r->ifcall.tread.offset,
                    r->ofcall.rread.count);
        }
    }

    ixp_respond(r, NULL);
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

void fs_update(IxpConn* c)
{
    c->aux = &_fs_p9srv;
    ixp_serve9conn(c);
}


