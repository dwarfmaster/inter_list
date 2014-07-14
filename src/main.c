
#include <stdio.h>
#include "spawn.h"

int main(int argc, char *argv[])
{
    spawn_t sp;
    char buffer[256];
    size_t rd;
    if(argc == 0)
        return 1;

    sp = spawn_create(argv + 1);
    if(!spawn_ok(sp)) {
        printf("Couldn't spawn argument.");
        return 1;
    }

    while((rd = spawn_read(sp, buffer, 256)) > 0) {
        buffer[rd] = '\0';
        printf("%s", buffer);
    }

    if(!spawn_ended(sp))
        spawn_wait(sp);
    spawn_close(&sp);

    return 0;
}

