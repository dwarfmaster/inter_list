
#include "events.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include "feeder.h"
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/ioctl.h>

#define EVENTS_MAX_SEQ 64

/* Modifiers */
#define EVENTS_MOD_SHIFT (1<<0)
#define EVENTS_MOD_ALTR  (1<<1)
#define EVENTS_MOD_CTRL  (1<<2)
#define EVENTS_MOD_ALTL  (1<<3)
#define EVENTS_MOD_SUPER (1<<6)
#define EVENTS_MOD_MASK  (EVENTS_MOD_SHIFT | EVENTS_MOD_CTRL \
        | EVENTS_MOD_ALTL | EVENTS_MOD_ALTR | EVENTS_MOD_SUPER)

/* Special keys. */
const char* _events_keys_name[] = {
    "up",
    "down",
    "left",
    "right",
    "home",
    "end",
    "return",
    "pagenext",
    "pageprev",
    NULL
};
int _events_keys_code[] = {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    (int)'\n',
    KEY_NPAGE,
    KEY_PPAGE,
};

/* Comp event (<C-A-l> for example). */
struct _events_comp_t {
    /* bitmask of the modifiers that required by this event. */
    unsigned char mods;
    /* The code of the letter. */
    int letter;
    /* The action to be executed when the event is completed. */
    char* action;
};

/* The comps events, sorted by letter. */
static struct _events_comp_t* _events_comps;
static size_t _events_comps_capa;
static size_t _events_comps_size;

/* Seq event (abc for example). */
struct _events_seq_t {
    /* The sequence of letters. */
    int* seq;
    /* Must a string be queried to the user. */
    bool use_prefix;
    /* The prefix to be displayed when querying a string to the user. */
    char* prefix;
    /* Action to be performat when the event is completed. */
    strformat_t* action;
};

/* The sequence events, sorted by seq. */
static struct _events_seq_t* _events_seqs;
static size_t _events_seqs_capa;
static size_t _events_seqs_size;
/* The symbols that can be parsed in the actions. */
static strformat_symbs_t* _events_sbs;

/* An array of the typed letters. */
static int _events_typed[EVENTS_MAX_SEQ];
/* The number of typed letters. */
static size_t _events_nb_typed;
/* Is the prompt on. */
static bool _events_inprompt;
/* Restrict where to search the seq event when a new key is typed. */
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

    _events_sbs = strformat_symbols("snti");
    if(!_events_sbs)
        return false;
    strformat_set(_events_sbs, 's', "");
    strformat_set(_events_sbs, 'n', "");
    strformat_set(_events_sbs, 'y', "");
    strformat_set(_events_sbs, 'i', "");

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

/* Add a new comp event. Returns false if the allocation failed : in this case,
 * all the comp events have been freed.
 */
static bool _events_comp_insert(size_t pos, struct _events_comp_t elem)
{
    size_t nsize = _events_comps_size + 1;
    size_t ncapa;
    if(nsize >= _events_comps_capa) {
        ncapa = _events_comps_capa + 10;
        _events_comps = realloc(_events_comps,
                ncapa * sizeof(struct _events_comp_t));
        if(!_events_comps) {
            _events_comps_size = 0;
            _events_comps_capa = 0;
            return false;
        }
    }

    memmove(_events_comps + pos + 1, _events_comps + pos,
            sizeof(struct _events_comp_t) * (_events_comps_size - pos));
    _events_comps[pos] = elem;
    ++_events_comps_size;
    return true;
}

/* Add a new seq event. Returns false if the allocation failed : in this case,
 * all the seq events have been freed.
 */
static bool _events_seq_insert(size_t pos, struct _events_seq_t elem)
{
    size_t nsize = _events_seqs_size + 1;
    size_t ncapa;
    if(nsize >= _events_seqs_capa) {
        ncapa = _events_seqs_capa + 10;
        _events_seqs = realloc(_events_seqs,
                ncapa * sizeof(struct _events_seq_t));
        if(!_events_seqs) {
            _events_seqs_size = 0;
            _events_seqs_capa = 0;
            return false;
        }
        _events_seqs_capa = ncapa;
    }

    memmove(_events_seqs + pos + 1, _events_seqs + pos,
            sizeof(struct _events_seq_t) * (_events_seqs_size - pos));
    _events_seqs[pos] = elem;
    ++_events_seqs_size;
    return true;
}

/* Parse the string of a comp event and add it. Return false if the syntax was
 * not respected.
 */
static bool _events_parse_comp(char* str, const char* action)
{
    struct _events_comp_t cp;
    size_t i;
    char c;
    bool letter = false;

    if(strlen(str) == 0 || strlen(str) % 2 != 1)
        return false;

    cp.mods = 0;
    for(i = 0; i < strlen(str); i += 2) {
        if(i != strlen(str) - 1 && str[i+1] != '-')
            return false;
        c = str[i];
        if(c == 'C')
            cp.mods |= EVENTS_MOD_CTRL;
        else if(c == 'A')
            cp.mods |= EVENTS_MOD_ALTR;
        else if(c == 'G')
            cp.mods |= EVENTS_MOD_ALTL;
        else if(c == 'S')
            cp.mods |= EVENTS_MOD_SHIFT;
        else if(c == 'W')
            cp.mods |= EVENTS_MOD_SUPER;
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
    if(!_events_comp_insert(i, cp)) {
        free(cp.action);
        return false;
    } else
        return true;
}

/* Returns the key code corresponding to a key name. */
static int _events_name_to_key(const char* name)
{
    size_t i;
    int key;
    char* used = strdup(name);

    for(i = 0; i < strlen(name); ++i) {
        if(used[i] >= 'A' && used[i] <= 'Z')
            used[i] = used[i] - 'A' + 'a';
    }

    key = -1;
    for(i = 0; _events_keys_name[i]; ++i) {
        if(strcmp(used, _events_keys_name[i]) == 0) {
            key = _events_keys_code[i];
            break;
        }
    }

    free(used);
    return key;
}

/* Parse a sequence of keys and returns a 0-terminated array of keycodes. */
static int* _events_parse_seq_string(const char* str)
{
    size_t i, j, size;
    int* ret;
    int key;
    bool inspe;
    char buffer[256];

    size = 0;
    ret = malloc(sizeof(int) * (strlen(str) + 1));
    if(!ret)
        return NULL;

    inspe = false;
    for(i = 0; i < strlen(str); ++i) {
        if(!inspe && str[i] != '[') {
            ret[size] = str[i];
            ++size;
        } else if(!inspe && str[i] == '[') {
            j = 0;
            inspe = true;
        } else if(inspe && str[i] == ']') {
            buffer[j] = '\0';
            key = _events_name_to_key(buffer);
            if(key >= 0) {
                ret[size] = key;
                ++size;
            }
            inspe = false;
        } else {
            buffer[j] = str[i];
            ++j;
        }
    }
    ret[size] = 0;
    ++size;

    /* Shrinking ret : can't fail. */
    ret = realloc(ret, size * sizeof(int));
    return ret;
}

/* Get the length of a 0-terminated array of keycodes. */
static size_t _events_seqlen(int* sq)
{
    size_t l = 0;
    while(sq[l++]);
    return --l;
}

/* Compare two array of 0-terminated array of keycodes. Same semantic as
 * strcmp.
 */
static int _events_seqcmp(int* sq1, int* sq2)
{
    size_t l1, l2, l, i;
    l1 = _events_seqlen(sq1);
    l2 = _events_seqlen(sq2);
    l  = (l1 < l2 ? l1 : l2);

    for(i = 0; i < l; ++i) {
        if(sq1[i] != sq2[i])
            return sq1[i] - sq2[i];
    }
    return l1 - l2;
}

/* Parse a complete seq event string and add it. */
static bool _events_parse_seq(char* str, const char* action)
{
    char* strtokbuf;
    struct _events_seq_t sq;
    size_t i;
    sq.use_prefix = false;
    sq.prefix     = NULL;

    if(strlen(str) == 0 || str[0] == '<')
        return false;

    i = strlen(str) - 1;
    if(str[i] == '>') {
        str[i] = '\0';
        str = strtok_r(str, "<", &strtokbuf);
        if(!str || strlen(str) == 0)
            return false;
        sq.seq = _events_parse_seq_string(str);
        str = strtok_r(NULL, "", &strtokbuf);
        if(!str) {
            free(sq.seq);
            return false;
        }
        sq.prefix = strdup(str);
        sq.use_prefix = true;
    }
    else
        sq.seq = _events_parse_seq_string(str);

    sq.action = strformat_parse(_events_sbs, action);
    for(i = 0; i < _events_seqs_size; ++i) {
        if(_events_seqcmp(sq.seq, _events_seqs[i].seq) < 0)
            break;
    }
    if(!_events_seq_insert(i, sq)) {
        if(sq.use_prefix)
            free(sq.prefix);
        free(sq.action);
        free(sq.seq);
        return false;
    } else
        return true;
}

/* Clear the buffer of already typed keys. */
static void _events_cancel()
{
    _events_nb_typed = 0;
    if(_events_inprompt) {
        curses_command_leave();
        _events_inprompt = false;
    }
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

/* Set the symbols for substitution in actions. */
static void _events_set_list_symbols()
{
    size_t id;
    char buffer[256];

    id = feeder_get_id();
    snprintf(buffer, 256, "%lu", id);
    strformat_set(_events_sbs, 'i', buffer);
    strformat_set(_events_sbs, 'n', feeder_get_name(id));
    strformat_set(_events_sbs, 't', feeder_get_text(id));
}

/* Get the actually pressed modifiers.
 * TODO doesn't work.
 */
static unsigned char _events_modifiers()
{
    unsigned char mods = 6;
    if(ioctl(0, TIOCLINUX, &mods) < 0)
        return 0;
    mods &= EVENTS_MOD_MASK;
    return mods;
}

/* Check if the pressed key validate a comp event. */
static bool _events_process_comp(int ev)
{
    unsigned char mods;
    size_t i;

    if(ev >= 'A' && ev <= 'Z')
        ev -= 'A';

    mods = _events_modifiers();
    for(i = 0; i < _events_comps_size; ++i) {
        if(_events_comps[i].mods == mods && _events_comps[i].letter == ev) {
            cmdparser_parse(_events_comps[i].action);
            return true;
        }
    }
    return false;
}

/* Check if the pressed key validate a seq event. */
static void _events_process_seq(int ev)
{
    size_t i;
    size_t off;
    struct _events_seq_t sq;

    off = _events_nb_typed;
    for(i = _events_typ_min; i < _events_typ_max; ++i) {
        if(_events_seqlen(_events_seqs[i].seq) > off) {
            if(_events_seqs[i].seq[off] < ev)
                ++_events_typ_min;
            else if(_events_seqs[i].seq[off] > ev)
                --_events_typ_max;
        }
        else
            --_events_typ_max;
    }

    /* No events matching. */
    if(_events_typ_max - _events_typ_min < 1)
        _events_cancel();
    /* One event potentially matching. */
    else {
        _events_typed[off]     = ev;
        _events_typed[off + 1] = 0;
        ++_events_nb_typed;
        for(i = _events_typ_min; i < _events_typ_max; ++i) {
            sq = _events_seqs[i];
            if(_events_seqcmp(_events_typed, sq.seq) == 0) {
                if(sq.use_prefix) {
                    _events_inprompt = true;
                    curses_command_enter(sq.prefix);
                    _events_typ_max = _events_typ_min = i;
                }
                else {
                    strformat_set(_events_sbs, 's', "");
                    _events_set_list_symbols();
                    cmdparser_parse(strformat_get(sq.action));
                    _events_cancel();
                }
            }
        }
    }
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
            _events_set_list_symbols();
            cmdparser_parse(
                strformat_get(_events_seqs[_events_typ_min].action));
            _events_cancel();
        }
    }
    else if(_events_nb_typed != 0 || !_events_process_comp(ev)) {}
        _events_process_seq(ev);
}

