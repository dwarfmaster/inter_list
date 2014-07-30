
#ifndef DEF_BARS
#define DEF_BARS

#include <stdbool.h>

bool bars_init();
void bars_quit();

/* Set the tob/bottom bars. They can include the following symbols :
 *  - %n : will be replaced by the name of the selected line.
 *  - %t : will be replaced by the text of the selected line.
 *  - %i : will be replaced by the id of the selected line.
 *  - %I : will be replaced by the total number of lines.
 */
bool bars_top_set(const char* br);
bool bars_bot_set(const char* br);

/* Update the values in the bars. */
void bars_update();

#endif

