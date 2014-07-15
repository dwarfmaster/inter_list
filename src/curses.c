
#include "curses.h"

/********************* Generic Ncurses abilities *****************************/
bool curses_init()
{
}

bool curses_end()
{
}

void curses_change_color(int c, uint8_t r, uint8_t g, uint8_t b)
{
}


/********************* List handling abilities *******************************/
void curses_list_color(int c)
{
}

void curses_list_clear()
{
}

bool curses_list_add_lines(size_t nb, char** lines)
{
}

bool curses_list_down(int nb)
{
}

bool curses_list_up(int nb)
{
}

size_t curses_list_get()
{
}

bool curses_list_set(size_t nb)
{
}

void curses_list_right(int nb)
{
}

bool curses_list_left(int nb)
{
}

bool curses_list_offset_reset()
{
}


/********************* Bars abilities ****************************************/
void curses_top_enable(bool e)
{
}

void curses_bot_enable(bool e)
{
}

void curses_top_set(const char* str)
{
}

void curses_bot_set(const char* str)
{
}

void curses_top_colors(int fg, int bg)
{
}

void curses_bot_colors(int fg, int bg)
{
}

void curses_command_enter(const char* prefix)
{
}

const char* curses_command_leave()
{
}

bool curses_command_must_leave()
{
}

