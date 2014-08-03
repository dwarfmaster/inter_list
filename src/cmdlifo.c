
#include "cmdlifo.h"
#include "spawn.h"
#include "cmdparser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct _cmdlifo_sp_t {
    spawn_t sp;
    char* buffer;
};
static struct _cmdlifo_sp_t* _cmdlifo_sps;
static size_t                _cmdlifo_nb;
static size_t                _cmdlifo_capa;
static bool                  _cmdlifo_spawned;

bool cmdlifo_init()
{
    _cmdlifo_nb      = 0;
    _cmdlifo_capa    = 10;
    _cmdlifo_sps     = malloc(_cmdlifo_capa * sizeof(struct _cmdlifo_sp_t));
    _cmdlifo_spawned = false;
    return (_cmdlifo_sps != NULL);
}

void cmdlifo_quit()
{
    size_t i;
    if(_cmdlifo_sps) {
        for(i = 0; i < _cmdlifo_nb; ++i) {
            if(_cmdlifo_sps[i].buffer)
                free(_cmdlifo_sps[i].buffer);
            spawn_close(&_cmdlifo_sps[i].sp);
        }
        free(_cmdlifo_sps);
    }
}

bool cmdlifo_push(const char* cmd)
{
    if(_cmdlifo_nb >= _cmdlifo_capa) {
        _cmdlifo_capa += 10;
        _cmdlifo_sps = realloc(_cmdlifo_sps, sizeof(spawn_t) * _cmdlifo_capa);
        if(!_cmdlifo_sps)
            return false;
    }

    if(_cmdlifo_nb != 0) {
        if(!spawn_ended(_cmdlifo_sps[_cmdlifo_nb - 1].sp))
            spawn_pause(_cmdlifo_sps[_cmdlifo_nb - 1].sp);
        else
            spawn_close(&_cmdlifo_sps[_cmdlifo_nb - 1].sp);
    }

    _cmdlifo_sps[_cmdlifo_nb].sp     = spawn_create_shell(cmd);
    _cmdlifo_sps[_cmdlifo_nb].buffer = NULL;
    if(!spawn_ok(_cmdlifo_sps[_cmdlifo_nb].sp)) {
        spawn_close(&_cmdlifo_sps[_cmdlifo_nb].sp);
        return false;
    }

    _cmdlifo_spawned = true;
    ++_cmdlifo_nb;
    return true;
}

static bool _cmdlifo_parse_buffer(char* buffer)
{
    char* line;
    char* strtokbuf;
    char* bufptr;
    size_t nb = _cmdlifo_nb - 1;
    _cmdlifo_spawned = false;

    line = strtok_r(buffer, "\n", &strtokbuf);
    while(line) {
        fprintf(stderr, "[%lu] %s\n", _cmdlifo_nb, line);
        cmdparser_parse(line);
        if(_cmdlifo_spawned) {
            line = strtok_r(NULL, "", &strtokbuf);
            bufptr = _cmdlifo_sps[nb].buffer;
            if(line)
                _cmdlifo_sps[nb].buffer = strdup(line);
            else
                _cmdlifo_sps[nb].buffer = NULL;
            if(bufptr)
                free(bufptr);
            _cmdlifo_spawned = false;
            return false;
        }
        line = strtok_r(NULL, "\n", &strtokbuf);
    }

    return true;
}

void cmdlifo_pop()
{
    if(_cmdlifo_nb == 0)
        return;

    --_cmdlifo_nb;
    spawn_close(&_cmdlifo_sps[_cmdlifo_nb].sp);
    if(_cmdlifo_sps[_cmdlifo_nb].buffer)
        free(_cmdlifo_sps[_cmdlifo_nb].buffer);

    if(_cmdlifo_nb != 0) {
        if(spawn_ok(_cmdlifo_sps[_cmdlifo_nb - 1].sp))
            spawn_resume(_cmdlifo_sps[_cmdlifo_nb - 1].sp);

        if(_cmdlifo_sps[_cmdlifo_nb - 1].buffer) {
            if(_cmdlifo_parse_buffer(_cmdlifo_sps[_cmdlifo_nb - 1].buffer)) {
                free(_cmdlifo_sps[_cmdlifo_nb - 1].buffer);
                _cmdlifo_sps[_cmdlifo_nb - 1].buffer = NULL;
            }
            else
                return;
        }

        if(spawn_ended(_cmdlifo_sps[_cmdlifo_nb - 1].sp))
            cmdlifo_pop();
    }
}

int cmdlifo_fd()
{
    if(_cmdlifo_nb == 0)
        return 0;
    else
        return spawn_fd(_cmdlifo_sps[_cmdlifo_nb - 1].sp);
}

void cmdlifo_update()
{
    char buffer[4096];
    size_t l;
    size_t nb = _cmdlifo_nb;

    if(nb == 0)
        return;
    --nb;

    while((l = spawn_read(_cmdlifo_sps[nb].sp, buffer, 4095)) != 0) {
        buffer[l] = '\0';
        if(!_cmdlifo_parse_buffer(buffer))
            return;
    }

    if(spawn_ended(_cmdlifo_sps[nb].sp))
        cmdlifo_pop();
}

