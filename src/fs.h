
#ifndef DEF_FS
#define DEF_FS

#include <stdbool.h>

/* Init the filesystem. */
bool fs_init();

/* Close the filesystem and free the ressources. */
void fs_close();

/* Get the FD to watch. */
int fs_fd();

/* Update the fs to apply the changes made to it and take into account the
 * internal changes.
 */
void fs_update();

#endif

