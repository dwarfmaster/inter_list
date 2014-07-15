
#ifndef DEF_CURSES
#define DEF_CURSES

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

/********************* Generic Ncurses abilities *****************************/
/* Initialises the curses display, must be called only once. */
bool curses_init();

/* End the curses display. */
bool curses_end();

/* Change the definition of a color. */
void curses_change_color(int c, uint8_t r, uint8_t g, uint8_t b);

/* Draw the changes on screen. */
void curses_draw();

/********************* List handling abilities *******************************/
/* Define the color of the background for the selected entry in the list. */
void curses_list_color(int c);

/* Clear the list. */
void curses_list_clear();

/* Add lines to the list. The lines won't be copied, so the given ones mustn't
 * be free'd by the caller.
 */
bool curses_list_add_lines(size_t nb, char** lines);

/* Move the selection downward by nb steps. Returns false if it reached the
 * end.
 */
bool curses_list_down(unsigned int nb);

/* Move the selection upward by nb steps. Returns false if it reached the
 * begginig.
 */
bool curses_list_up(unsigned int nb);

/* Get the number of the selected line. */
size_t curses_list_get();

/* Set the number of the selected line. Return false if the line is invalid. */
bool curses_list_set(size_t nb);

/* Move the screen to the right by nb chars. */
void curses_list_right(unsigned int nb);

/* Move the screen to the left by nb chars. Returns left if it reached the
 * beggining.
 */
bool curses_list_left(unsigned int nb);

/* Reset the list to the left. */
void curses_list_offset_reset();

/********************* Bars abilities ****************************************/
/* Set the content of the top/bottom bars. The strings will be deduplicated. If
 * they are NULL, the top/bottom bars will be disabled. Returns true if
 * curses_list_redraw must be called.
 */
bool curses_top_set(const char* str);
bool curses_bot_set(const char* str);

/* Set the fg and bg colors of the top/bottom bars. */
void curses_top_colors(int fg, int bg);
void curses_bot_colors(int fg, int bg);

/* Start the command line bar. The prefix will be printed at the beggining of
 * the command line.
 */
void curses_command_enter(const char* prefix);

/* Leave the command line bar : it will be cleared and its contents will be
 * returned. They mustn't be free'd and will be valid up until the next call to
 * curses_command_enter.
 */
const char* curses_command_leave();

/* Parse an event. Returns false if the command line must be left.
 * TODO handle utf8
 */
bool curses_command_parse_event(char c);

#endif

