
#include "curses.h"
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>

#define CURSES_TEXT_LENGTH 512

/* Global ncurses variables. */
static uint16_t _curses_term_width;
static uint16_t _curses_term_height;

/* The list. */
static size_t _curses_list_nb;
static size_t _curses_list_capacity;
static char** _curses_list_lines;
static size_t _curses_list_sel;
static size_t _curses_list_offset;

/* Top and bottom bars. */
static bool  _curses_top_enable;
static char* _curses_top_str;
static bool  _curses_bot_enable;
static char* _curses_bot_str;
static int   _curses_top_fg;
static int   _curses_top_bg;
static int   _curses_bot_fg;
static int   _curses_bot_bg;

/* Command line. */
static bool        _curses_cmd_in;
static const char* _curses_cmd_prefix;
static char        _curses_cmd_text[CURSES_TEXT_LENGTH];
static size_t      _curses_cmd_pos;

/********************* Generic Ncurses abilities *****************************/
bool curses_init()
{
    /* Initialising ncurses. */
    if(!initscr())
        return false;
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* TODO Init colors */

    _curses_term_width  = COLS;
    _curses_term_height = LINES;

    /* Initialising the list. */
    _curses_list_nb       = 0;
    _curses_list_capacity = 10;
    _curses_list_sel      = 0;
    _curses_list_offset   = 0;
    _curses_list_lines    = malloc(sizeof(char*) * 10);
    if(!_curses_list_lines)
        return false;

    /* Initialising the top and bottom bars. */
    _curses_top_enable = false;
    _curses_top_str    = NULL;
    _curses_bot_enable = false;
    _curses_bot_str    = NULL;
    _curses_top_fg     = COLOR_BLACK;
    _curses_top_bg     = COLOR_WHITE;
    _curses_bot_fg     = COLOR_BLACK;
    _curses_bot_bg     = COLOR_WHITE;

    /* Initialising the command line. */
    _curses_cmd_in     = false;
    _curses_cmd_prefix = "";

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
    /* TODO clear colors */
    return true;
}

void curses_change_color(int c, uint8_t r, uint8_t g, uint8_t b)
{
    /* TODO */
}

void curses_draw()
{
    refresh();
}

/********************* List handling abilities *******************************/
void curses_list_color(int c)
{
    /* TODO */
}

void curses_list_clear()
{
    _curses_list_nb     = 0;
    _curses_list_sel    = 0;
    _curses_list_offset = 0;
    curses_list_redraw();
}

bool curses_list_add_lines(size_t nb, char** lines)
{
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
    _curses_list_offset += nb;
    curses_list_redraw();
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
    curses_list_redraw();
    return ret;
}

void curses_list_offset_reset()
{
    _curses_list_offset = 0;
    curses_list_redraw();
}

void curses_list_redraw()
{
    /* TODO */
}

/********************* Bars abilities ****************************************/
bool curses_top_set(const char* str)
{
    if(_curses_top_str)
        free(_curses_top_str);

    if(str) {
        _curses_top_str = strdup(str);
        /* TODO draw the bar. */
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
        /* TODO draw the bar. */
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
    _curses_top_fg = fg;
    _curses_top_bg = bg;
}

void curses_bot_colors(int fg, int bg)
{
    _curses_bot_fg = fg;
    _curses_bot_bg = bg;
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
    _curses_cmd_in = false;
    return _curses_cmd_text;
}

bool curses_command_parse_event(char c)
{
    if(isprint(c)) {
        _curses_cmd_text[_curses_cmd_pos] = c;
        ++_curses_cmd_pos;
        _curses_cmd_text[_curses_cmd_pos] = '\0';
        return true;
    }
    else if(c == '\n')
        return false;
    else
        return true;
}

