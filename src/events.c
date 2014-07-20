
#include "events.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define EVENTS_MAX_SEQ 64

/* Comp event (<C-A-l> for example). */
struct _events_comp_t {
    bool ctrl;
    bool shift;
    bool alt;
    int letter;
    char* action;
};
/* The comps events, sorted by letter. */
static struct _events_comp_t* _events_comps;
static size_t _events_comps_capa;
static size_t _events_comps_size;

/* Seq event (abc for example). */
struct _events_seq_t {
    char* seq;
    bool use_prefix;
    char* prefix;
    strformat_t* action;
};
/* The sequence events, sorted by seq. */
static struct _events_seq_t* _events_seqs;
static size_t _events_seqs_capa;
static size_t _events_seqs_size;
static strformat_symbs_t* _events_sbs;

/* Handling events. */
static char _events_typed[EVENTS_MAX_SEQ];
static size_t _events_nb_typed;
static bool _events_inprompt;
static size_t _events_typ_min;
static size_t _events_typ_max;

bool events_init()
{
    _events_comps = NULL;
    _events_seqs  = NULL;
    _events_sbs   = NULL;

    _events_comps_size = 0;
    _events_comps_capa = 10;
    _events_comps = malloc(sizeof(struct _events_comp_t) * _events_comps_capa);
    if(!_events_comps)
        return false;

    _events_seqs_size = 0;
    _events_seqs_capa = 10;
    _events_seqs = malloc(sizeof(struct _events_seq_t) * _events_seqs_capa);
    if(!_events_seqs)
        return false;

    _events_sbs = strformat_symbols("s");
    if(!_events_sbs)
        return false;
    strformat_set(_events_sbs, 's', "");

    _events_nb_typed = 0;
    _events_typ_min  = 0;
    _events_typ_max  = 0;

    return true;
}

void events_quit()
{
    size_t i;

    if(_events_comps) {
        for(i = 0; i < _events_comps_size; ++i)
            free(_events_comps[i].action);
        free(_events_comps);
    }

    if(_events_seqs) {
        for(i = 0; i < _events_seqs_size; ++i) {
            free(_events_seqs[i].seq);
            strformat_destroy(_events_seqs[i].action);
            if(_events_seqs[i].use_prefix)
                free(_events_seqs[i].prefix);
        }
        free(_events_seqs);
    }
    
    if(_events_sbs)
        strformat_symbols_destroy(_events_sbs);
}

void events_clear()
{
    _events_comps_size = 0;
    _events_seqs_size  = 0;
}

static bool _events_insert(void** array, size_t pos, size_t* capa,
        size_t* size, size_t elem_size)
{
    size_t nsize = *size + 1;
    size_t ncapa;
    if(nsize >= *capa) {
        ncapa = *capa + 10;
        *array = realloc(*array, ncapa * elem_size);
        if(!*array) {
            *size = 0;
            *capa = 0;
            return false;
        }
    }

    memmove(*array + pos + 1, *array + pos, elem_size * (*size - pos));
    return true;
}

static bool _events_parse_comp(char* str, const char* action)
{
    struct _events_comp_t cp;
    size_t i;
    char c;
    bool letter = false;

    if(strlen(str) == 0 || strlen(str) % 2 != 1)
        return false;

    for(i = 0; i < strlen(str); i += 2) {
        if(i != strlen(str) - 1 && str[i+1] != '-')
            return false;
        c = str[i];
        if(c == 'C')
            cp.ctrl = true;
        else if(c == 'A')
            cp.alt = true;
        else if(c == 'S')
            cp.shift = true;
        else if(!letter) {
            cp.letter = c;
            letter = true;
        }
        else
            return false;
    }
    if(!letter)
        return false;

    cp.action = strdup(action);
    for(i = 0; i < _events_comps_size; ++i) {
        if(cp.letter < _events_comps[i].letter)
            break;
    }
    if(!_events_insert((void**)&_events_comps, i, &_events_comps_capa,
                &_events_comps_size, sizeof(struct _events_comp_t))) {
        free(cp.action);
        return false;
    } else {
        _events_comps[i] = cp;
        return true;
    }
}

static bool _events_parse_seq(char* str, const char* action)
{
    struct _events_seq_t sq;
    size_t i;
    sq.use_prefix = false;
    sq.prefix     = NULL;

    if(strlen(str) == 0 || str[0] == '<')
        return false;

    i = strlen(str) - 1;
    if(str[i] == '>') {
        str[i] = '\0';
        str = strtok(str, "<");
        if(!str || strlen(str) == 0)
            return false;
        sq.seq = strdup(str);
        str = strtok(NULL, "");
        if(!str) {
            free(sq.seq);
            return false;
        }
        sq.prefix = strdup(str);
        sq.use_prefix = true;
    }
    else
        sq.seq = strdup(str);

    sq.action = strformat_parse(_events_sbs, action);
    for(i = 0; i < _events_seqs_size; ++i) {
        if(strcmp(sq.seq, _events_seqs[i].seq) < 0)
            break;
    }
    if(!_events_insert((void**)&_events_seqs, i, &_events_seqs_capa,
                &_events_seqs_size, sizeof(struct _events_seq_t))) {
        if(sq.use_prefix)
            free(sq.prefix);
        free(sq.action);
        free(sq.seq);
        return false;
    } else {
        _events_seqs[i] = sq;
        return true;
    }
}

static void _events_cancel()
{
    _events_nb_typed = 0;
    if(_events_inprompt)
        curses_command_leave();
    _events_typ_min = 0;
    _events_typ_max = _events_seqs_size;
}

bool events_add(const char* ev, const char* action)
{
    char* parsed;
    size_t i;
    bool ret = true;

    parsed = strdup(ev);
    if(strlen(parsed) == 0)
        ret = false;
    else if(parsed[0] == '<') {
        i = strlen(parsed) - 1;
        if(parsed[i] != '>')
            ret = false;
        else {
            parsed[i] = '\0';
            ret = _events_parse_comp(parsed + 1, action);
        }
    }
    else
        ret = _events_parse_seq(parsed, action);

    free(parsed);
    _events_cancel();
    return ret;
}

static bool _events_process_comp(int ev)
{
    /* TODO */
}

static void _events_process_seq(int ev)
{
    /* TODO */
}

void events_process()
{
    int ev;

    ev = getch();
    if(ev == KEY_CANCEL)
        _events_cancel();
    else if(_events_inprompt) {
        if(!curses_command_parse_event(ev)) {
            strformat_set(_events_sbs, 's', curses_command_leave());
            cmdparser_parse(
                strformat_get(_events_seqs[_events_typ_min].action));
            _events_cancel();
        }
    }
    else if(_events_nb_typed == 0 && _events_process_comp(ev)) {}
    else
        _events_process_seq(ev);
}

