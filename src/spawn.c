
#include "spawn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static const char* _spawn_get_shell()
{
    const char* sh = getenv("SHELL");
    return (sh ? sh : "/bin/sh");
}

spawn_t spawn_create_shell(const char* command)
{
    char* argv[4];
    spawn_t sp;

    argv[0] = strdup(_spawn_get_shell());
    argv[1] = "-c";
    argv[2] = strdup(command);
    argv[3] = NULL;
    sp = spawn_create(argv);

    free(argv[0]);
    free(argv[2]);
    return sp;
}

bool spawn_exec(char* const prog[])
{
    pid_t pid = vfork();

    if(pid < 0)
        return false;
    else if(pid == 0) {
        execv(prog[0], prog);
        exit(0);
    }
    else
        waitpid(pid, NULL, 0);

    return true;
}

bool spawn_exec_shell(const char* command)
{
    char* argv[4];
    argv[0] = strdup(_spawn_get_shell());
    argv[1] = "-c";
    argv[2] = strdup(command);
    argv[3] = NULL;
    return spawn_exec(argv);
}

spawn_t spawn_init()
{
    spawn_t sp;
    sp.process = -1;
    return sp;
}

bool spawn_ok(spawn_t sp)
{
    return (sp.process >= 0);
}

void spawn_pause(spawn_t sp)
{
    if(spawn_paused(sp))
        return;
    kill(sp.process, SIGSTOP);
}

void spawn_resume(spawn_t sp)
{
    if(!spawn_paused(sp))
        return;
    kill(sp.process, SIGCONT);
}

bool spawn_paused(spawn_t sp)
{
    int stat_loc = 0;
    if(!spawn_ok(sp))
        return true;
    waitpid(sp.process, &stat_loc, WNOHANG);
    return WIFSTOPPED(stat_loc);
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
    int stat_loc = 0;
    if(!spawn_ok(sp))
        return true;

    waitpid(sp.process, &stat_loc, WNOHANG);
    return WIFEXITED(stat_loc);
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


