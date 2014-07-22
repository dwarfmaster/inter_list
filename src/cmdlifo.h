
#ifndef DEF_CMDLIFO
#define DEF_CMDLIFO

#include <stdbool.h>

/* Init and free the cmd reader lifo structure. */
bool cmdlifo_init();
void cmdlifo_quit();

/* Push a spawn on top of the lifo structure. */
bool cmdlifo_push(const char* cmd);

/* Remove and close the top spawned. */
void cmdlifo_pop();

/* Get the fd associated to the top of the lifo. */
int cmdlifo_fd();

/* Read the input and update the state. */
void cmdlifo_update();

#endif

