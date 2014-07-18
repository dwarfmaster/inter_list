
#ifndef DEF_EVENTS
#define DEF_EVENTS

#include <stdbool.h>

/* Init the events. */
bool events_init();

/* Free the events. */
void events_quit();

/* Clear the events. */
void events_clear();

/* Add an event. ev is the string describing the event. action must be a valid
 * command. ev must have one of the following syntaxes :
 *  - <C-l>      : press of letter l while ctrl mod is on. The handled
 *                 modifiers are ctrl (C), shift (S) and alt (A). You can
 *                 combine them (<S-A-c> for example), but there can be only
 *                 one letter.
 *  - abc        : if the letters a, b and then c are pressed in this order.
 *                 This is case insensitive.
 *  - abc<Cmd :> : the event is the same as before, but when triggered a prompt
 *                 is opened to query a string from the user.
 * If action contain %s, it will be replaced by the string queried to the user.
 */
bool events_add(const char* ev, const char* action);

/* Non-blocking event processing. */
void events_process();

#endif

