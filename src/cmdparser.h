
#ifndef DEF_CMDPARSER
#define DEF_CMDPARSER

#include <stdbool.h>

/* The type of callback that is called when a command is parsed. The first
 * is a string with the arguments of the command, the second is a specific data
 * given by the caller.
 */
typedef void (*cmdparser_callback_t)(const char*, void*);

/* Init the command parser. */
bool cmdparser_init();

/* Free the command parser. */
void cmdparser_quit();

/* Add a command. name won't be deduplicated, so it must stay valid until the
 * call to cmdparser_quit. It returns false if the command is already
 * configured.
 */
bool cmdparser_add_command(const char* name,
        cmdparser_callback_t cb, void* data);

/* Change the data associated to a callback. Returns false if name doesn't
 * refer to a configured command.
 */
bool cmdparser_set_data(const char* name, void* data);

/* Change the callback for a command. Returns false if name doesn't refer to a
 * configured command.
 */
bool cmdparser_change_callback(const char* name,
        cmdparser_callback_t cb, void* data);

/* Parse an entry line. The line must have the format 'command args'. Returns
 * false if command isn't recognised. The callback associated with the command
 * will be called, getting the args as its first argument. If there is no args,
 * it will receive an empty string. line won't be stored, so it doesn't need to
 * remain valid after the call.
 */
bool cmdparser_parse(const char* line);

#endif

