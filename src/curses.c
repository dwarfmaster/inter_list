
#include "curses.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <ncurses.h>

#define CURSES_TEXT_LENGTH 512

/* Global ncurses variables. */
static uint16_t _curses_term_width;
static uint16_t _curses_term_height;
static bool     _curses_colors;

/* The list. */
static size_t       _curses_list_nb;
static size_t       _curses_list_capacity;
static const char** _curses_list_lines;
static size_t       _curses_list_first;
static size_t       _curses_list_sel;
static size_t       _curses_list_offset;
static int          _curses_list_color;
static bool         _curses_list_mustdraw;

/* Top and bottom bars. */
static bool  _curses_top_enable;
static char* _curses_top_str;
static bool  _curses_top_mustdraw;

static bool  _curses_bot_enable;
static char* _curses_bot_str;
static bool  _curses_bot_mustdraw;

/* Command line. */
static bool        _curses_cmd_in;
static const char* _curses_cmd_prefix;
static char        _curses_cmd_text[CURSES_TEXT_LENGTH];
static size_t      _curses_cmd_pos;
static bool        _curses_cmd_mustdraw;

/********************* Generic Ncurses abilities *****************************/
bool curses_init()
{
    /* Initialising ncurses. */
    if(!initscr())
        return false;
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    _curses_term_width  = COLS;
    _curses_term_height = LINES;

    /* Initialising colors. */
    _curses_colors = has_colors();
    if(_curses_colors)
        start_color();

    /* Initialising the list. */
    _curses_list_nb       = 0;
    _curses_list_capacity = 10;
    _curses_list_sel      = 0;
    _curses_list_first    = 0;
    _curses_list_offset   = 0;
    _curses_list_lines    = malloc(sizeof(char*) * 10);
    if(!_curses_list_lines)
        return false;

    /* Initialising the top and bottom bars. */
    _curses_top_enable = false;
    _curses_top_str    = NULL;
    _curses_bot_enable = false;
    _curses_bot_str    = NULL;

    /* Initialising top (1), bottom (2), cmd line(3), list sel (4) and list
     * item (5).
     */
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_WHITE);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);

    /* Initialising the command line. */
    _curses_cmd_in     = false;
    _curses_cmd_prefix = "";

    /* Preparing drawing. */
    _curses_list_mustdraw = true;
    _curses_top_mustdraw  = true;
    _curses_bot_mustdraw  = true;
    _curses_cmd_mustdraw  = true;

    return true;
}

bool curses_end()
{
    endwin();
    if(_curses_list_lines)
        free(_curses_list_lines);
    if(_curses_top_str)
        free(_curses_top_str);
    if(_curses_bot_str)
        free(_curses_bot_str);
    return true;
}

static void _curses_draw_line(const char* text, unsigned int y, int cp)
{
    size_t i;
    char* txt;

    /* First fill the the bg with spaces. */
    attron(COLOR_PAIR(cp));
    for(i = 0; i < _curses_term_width; ++i)
        mvaddch(y, i, ' ');

    /* Print the text. */
    txt = strdup(text);
    if(strlen(txt) > _curses_term_width)
        txt[_curses_term_width] = '\0';
    mvprintw(y, 0, "%s", txt);
    free(txt);
}

static void _curses_list_draw()
{
    size_t i, id;
    unsigned int lines;
    unsigned int yoff;
    int cp;

    lines = _curses_term_height - 1
        - (_curses_top_enable ? 1 : 0)
        - (_curses_bot_enable ? 1 : 0);
    if(_curses_list_first + lines >= _curses_list_nb)
        lines = _curses_list_nb - _curses_list_first;

    yoff = (_curses_top_enable ? 1 : 0);
    for(i = 0; i < lines; ++i) {
        id = i + _curses_list_first;
        if(id == _curses_list_sel)
            cp = 4;
        else
            cp = 5;

        if(strlen(_curses_list_lines[id]) <= _curses_list_offset)
            _curses_draw_line("", yoff + i, cp);
        else {
            _curses_draw_line(_curses_list_lines[id] + _curses_list_offset,
                    yoff + i, cp);
        }
    }
}

void curses_draw()
{
    if(_curses_list_mustdraw) {
        _curses_list_draw();
        _curses_list_mustdraw = false;
    }

    if(_curses_top_mustdraw) {
        _curses_draw_line(_curses_top_str, 0, 1);
        _curses_top_mustdraw = false;
    }

    if(_curses_bot_mustdraw) {
        _curses_draw_line(_curses_bot_str, _curses_term_height - 2, 2);
        _curses_cmd_mustdraw = false;
    }

    if(_curses_cmd_mustdraw) {
        /* TODO */
        _curses_cmd_mustdraw = false;
    }

    move(_curses_term_height - 1, 0);
    refresh();
}

/********************* List handling abilities *******************************/
void curses_list_color(int c)
{
    _curses_list_color = c;
    /* TODO redraw selected line. */
}

void curses_list_clear()
{
    _curses_list_nb       = 0;
    _curses_list_sel      = 0;
    _curses_list_first    = 0;
    _curses_list_offset   = 0;
    _curses_list_mustdraw = true;
}

bool curses_list_add_lines(size_t nb, const char** lines)
{
    size_t nsize = _curses_list_nb + nb;
    size_t ncapa;
    size_t i;
    void* temp;

    if(nsize >= _curses_list_capacity) {
        /* ((nsize + 9) / 10) is ceil(nsize / 10) */
        ncapa = 10 * ((nsize + 9) / 10) + 10;
        temp = realloc(_curses_list_lines, ncapa * sizeof(const char*));
        if(!temp)
            return false;
        _curses_list_lines = temp;
        _curses_list_capacity = ncapa;
    }

    for(i = 0; i < nb; ++i)
        _curses_list_lines[_curses_list_nb + i] = lines[i];
    _curses_list_nb = nsize;

    /* TODO draw new lines if necessary */
    return true;
}

bool curses_list_down(unsigned int nb)
{
    /* TODO drawing. */
    _curses_list_sel += nb;
    if(_curses_list_sel >= _curses_list_nb) {
        _curses_list_sel = _curses_list_nb - 1;
        return false;
    }
    return true;
}

bool curses_list_up(unsigned int nb)
{
    /* TODO drawing. */
    if(_curses_list_sel < nb) {
        _curses_list_sel = 0;
        return false;
    }
    _curses_list_sel -= nb;
    return true;
}

size_t curses_list_get()
{
    return _curses_list_sel;
}

bool curses_list_set(size_t nb)
{
    /* TODO drawing. */
    if(nb >= _curses_list_nb)
        return false;
    _curses_list_sel = nb;
    return true;
}

void curses_list_right(unsigned int nb)
{
    _curses_list_offset  += nb;
    _curses_list_mustdraw = true;
}

bool curses_list_left(unsigned int nb)
{
    bool ret;
    if(_curses_list_offset < nb) {
        _curses_list_offset = 0;
        ret = false;
    }
    else {
        _curses_list_offset -= nb;
        ret = true;
    }
    _curses_list_mustdraw = true;
    return ret;
}

void curses_list_offset_reset()
{
    _curses_list_offset   = 0;
    _curses_list_mustdraw = true;
}

/********************* Bars abilities ****************************************/
bool curses_top_set(const char* str)
{
    if(_curses_top_str)
        free(_curses_top_str);

    if(str) {
        _curses_top_str = strdup(str);
        _curses_top_mustdraw = true;
        if(!_curses_top_enable) {
            _curses_top_enable = true;
            return true;
        }
    }
    else {
        _curses_top_str = NULL;
        if(_curses_top_enable) {
            _curses_top_enable = false;
            return true;
        }
    }
    return false;
}

bool curses_bot_set(const char* str)
{
    if(_curses_bot_str)
        free(_curses_bot_str);

    if(str) {
        _curses_bot_str = strdup(str);
        _curses_bot_mustdraw = true;
        if(!_curses_bot_enable) {
            _curses_bot_enable = true;
            return true;
        }
    }
    else {
        _curses_bot_str = NULL;
        if(_curses_bot_enable) {
            _curses_bot_enable = false;
            return true;
        }
    }
    return false;
}

void curses_top_colors(int fg, int bg)
{
    init_pair(1, fg, bg);
    _curses_top_mustdraw = true;
}

void curses_bot_colors(int fg, int bg)
{
    init_pair(2, fg, bg);
    _curses_bot_mustdraw = true;
}

void curses_command_enter(const char* prefix)
{
    _curses_cmd_in      = true;
    _curses_cmd_prefix  = prefix;
    _curses_cmd_text[0] = '\0';
    _curses_cmd_pos     = 0;
}

const char* curses_command_leave()
{
    if(!_curses_cmd_in)
        return "";
    _curses_cmd_in       = false;
    _curses_cmd_mustdraw = true;
    return _curses_cmd_text;
}

bool curses_command_parse_event(char c)
{
    if(isprint(c)) {
        _curses_cmd_text[_curses_cmd_pos] = c;
        ++_curses_cmd_pos;
        _curses_cmd_text[_curses_cmd_pos] = '\0';
        _curses_cmd_mustdraw = true;
        return true;
    }
    else if(c == '\n')
        return false;
    else
        return true;
}

