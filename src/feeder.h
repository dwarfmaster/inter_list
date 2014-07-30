
#ifndef DEF_FEEDER
#define DEF_FEEDER

#include <stdbool.h>
#include <stdlib.h>

/* Init and free the feeder. */
bool feeder_init();
void feeder_quit();

/* Set the feeding command : clear any previous content. */
bool feeder_set(const char* command);

/* Get the fd to watch. */
int feeder_fd();

/* Read data from the feeder and add it to the list. */
void feeder_update();

/* Get the id of the selection. */
size_t feeder_get_id();

/* Get the number of lines. */
size_t feeder_get_lines();

/* Get the name of a line. Returns NULL if id is invalid. */
const char* feeder_get_name(size_t id);

/* Get the text of a line. Returns NULL if id is invalid. */
const char* feeder_get_text(size_t id);

#endif

