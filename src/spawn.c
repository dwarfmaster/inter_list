
#include "spawn.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

spawn_t spawn_create(char* const prog[])
{
    spawn_t sp;
    sp.process = -1;

    /* Creating the pipe. */
    if(pipe(sp.pipe) < 0)
        return sp;

    /* Forking. */
    sp.process = fork();
    if(sp.process < 0) {
        close(sp.pipe[0]);
        close(sp.pipe[1]);
        return sp;
    }

    /* Child. */
    if(sp.process == 0) {
        close(sp.pipe[0]);
        dup2(sp.pipe[1], 1); /* Connecting stdout to pipe. */
        execvp(prog[0], prog);
        close(sp.pipe[1]);
        exit(EXIT_SUCCESS);
    }

    /* Parent. */
    close(sp.pipe[1]);
    return sp;
}

bool spawn_ok(spawn_t sp)
{
    return (sp.process >= 0);
}

void spawn_close(spawn_t* sp)
{
    if(!spawn_ok(*sp))
        return;
    if(!spawn_ended(*sp)) {
        kill(sp->process, SIGKILL);
        spawn_wait(*sp); /* Prevent it from blocking. */
    }
    sp->process = -1;
    close(sp->pipe[0]);
}

void spawn_wait(spawn_t sp)
{
    if(!spawn_ok(sp) || spawn_ended(sp))
        return;
    waitpid(sp.process, NULL, 0);
}

bool spawn_ended(spawn_t sp)
{
    char buffer[256];
    if(!spawn_ok(sp))
        return true;

    snprintf(buffer, 256, "/proc/%i", sp.process);
    return (access(buffer, F_OK) == -1);
}

size_t spawn_read(spawn_t sp, char* buffer, size_t bufsize)
{
    if(!spawn_ok(sp))
        return 0;
    return read(sp.pipe[0], buffer, bufsize);
}

bool spawn_ready(spawn_t sp)
{
    if(!spawn_ok(sp))
        return false;
    return (read(sp.pipe[0], NULL, 0) == 0);
}

int spawn_fd(spawn_t sp)
{
    if(!spawn_ok(sp))
        return -1;
    return sp.pipe[0];
}


