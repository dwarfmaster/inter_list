
#ifndef DEF_FS
#define DEF_FS

#include <stdbool.h>
#include <ixp.h>

/* Init the filesystem. Open the unix socket specified by path. */
bool fs_init(const char* path);

/* Close the filesystem and free the ressources. */
void fs_close();

/* Get the FD to watch. */
int fs_fd();

/* Get the aux pointer to give to ixp_listen for fs_fd() */
void* fs_aux();

/* Callback to bind to fs_fd() in ixp mainloop. */
void fs_update(IxpConn* c);

#endif

