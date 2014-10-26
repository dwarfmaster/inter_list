
#include "fs.h"
#include <ixp.h>

/* The fd of the server. */
static int _fs_fd;

bool fs_init()
{
    _fs_fd = 0;
    /* TODO Initialisation process. */
    return false;
}

void fs_close()
{
    /* TODO */
}

int fs_fd()
{
    return _fs_fd;
}

void fs_update()
{
    /* TODO write the filesystem. */
}


