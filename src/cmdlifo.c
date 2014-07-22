
#include "cmdlifo.h"
#include "spawn.h"
#include "cmdparser.h"
#include <stdlib.h>
#include <string.h>

static spawn_t* _cmdlifo_sps;
static size_t   _cmdlifo_nb;
static size_t   _cmdlifo_capa;

bool cmdlifo_init()
{
    _cmdlifo_nb   = 0;
    _cmdlifo_capa = 10;
    _cmdlifo_sps  = malloc(_cmdlifo_capa * sizeof(spawn_t));
    return (_cmdlifo_sps != NULL);
}

void cmdlifo_quit()
{
    if(_cmdlifo_sps)
        free(_cmdlifo_sps);
}

bool cmdlifo_push(const char* cmd)
{
    if(_cmdlifo_nb >= _cmdlifo_capa) {
        _cmdlifo_capa += 10;
        _cmdlifo_sps = realloc(_cmdlifo_sps, sizeof(spawn_t) * _cmdlifo_capa);
        if(!_cmdlifo_sps)
            return false;
    }

    if(_cmdlifo_nb != 0)
        spawn_pause(_cmdlifo_sps[_cmdlifo_nb - 1]);
    _cmdlifo_sps[_cmdlifo_nb] = spawn_create_shell(cmd);
    if(!spawn_ok(_cmdlifo_sps[_cmdlifo_nb])) {
        spawn_close(&_cmdlifo_sps[_cmdlifo_nb]);
        return false;
    }

    ++_cmdlifo_nb;
    return true;
}

void cmdlifo_pop()
{
    if(_cmdlifo_nb == 0)
        return;

    --_cmdlifo_nb;
    spawn_close(&_cmdlifo_sps[_cmdlifo_nb]);
    if(_cmdlifo_nb != 0)
        spawn_resume(_cmdlifo_sps[_cmdlifo_nb - 1]);
}

int cmdlifo_fd()
{
    if(_cmdlifo_nb == 0)
        return 0;
    else
        return spawn_fd(_cmdlifo_sps[_cmdlifo_nb - 1]);
}

void cmdlifo_update()
{
    char buffer[4096];
    char* line;
    size_t l;

    if(_cmdlifo_nb == 0)
        return;

    if(spawn_ended(_cmdlifo_sps[_cmdlifo_nb - 1])) {
        cmdlifo_pop();
        cmdlifo_update();
    }

    while((l = spawn_read(_cmdlifo_sps[_cmdlifo_nb - 1], buffer, 4095)) != 0) {
        buffer[l] = '\0';
        line = strtok(buffer, "\n");
        while(line) {
            cmdparser_parse(line);
            line = strtok(NULL, "\n");
        }
    }
}

