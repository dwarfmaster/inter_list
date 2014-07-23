
#ifndef DEF_FEEDER
#define DEF_FEEDER

#include <stdbool.h>

/* Init and free the feeder. */
bool feeder_init();
void feeder_quit();

/* Set the feeding command : clear any previous content. */
bool feeder_set(const char* command);

/* Get the fd to watch. */
int feeder_fd();

/* Read data from the feeder and add it to the list. */
void feeder_update();

#endif

