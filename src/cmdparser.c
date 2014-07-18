
#include "cmdparser.h"
#include <string.h>
#include <stdlib.h>

struct _cmdparser_command_t {
    const char* name;
    cmdparser_callback_t cb;
    void* data;
};

/* The array of commands. */
static struct _cmdparser_command_t* _cmdparser_cmds;
/* The number of commands. */
static size_t _cmdparser_nb;
/* The size of the array. */
static size_t _cmdparser_capa;

bool cmdparser_init()
{
    _cmdparser_nb   = 0;
    _cmdparser_capa = 10;
    _cmdparser_cmds
        = malloc(_cmdparser_capa * sizeof(struct _cmdparser_command_t));
    if(!_cmdparser_cmds) {
        _cmdparser_capa = 0;
        return false;
    }
    return true;
}

void cmdparser_quit()
{
    if(_cmdparser_cmds)
        free(_cmdparser_cmds);
}

static bool _cmdparser_insert(size_t id, struct _cmdparser_command_t cmd)
{
    if(id != _cmdparser_nb) {
        memmove(_cmdparser_cmds + id + 1,
                _cmdparser_cmds + id,
                sizeof(struct _cmdparser_command_t) * (_cmdparser_nb - id));
    }
    _cmdparser_cmds[id] = cmd;
    return true;
}

bool cmdparser_add_command(const char* name,
        cmdparser_callback_t cb, void* data)
{
    size_t id;
    struct _cmdparser_command_t cmd;

    /* Resize if necessary. */
    if(_cmdparser_nb + 1 == _cmdparser_capa) {
        _cmdparser_capa += 10;
        _cmdparser_cmds = realloc(_cmdparser_cmds,
                _cmdparser_capa * sizeof(struct _cmdparser_command_t));
        if(!_cmdparser_cmds) {
            _cmdparser_capa = 0;
            _cmdparser_nb   = 0;
            return false;
        }
    }

    /* Insert it. */
    id = 0;
    while(id < _cmdparser_nb && strcmp(_cmdparser_cmds[id].name, name) < 0) ++id;
    cmd.name = name;
    cmd.cb   = cb;
    cmd.data = data;
    _cmdparser_insert(id, cmd);
    ++_cmdparser_nb;
    return true;
}

struct _cmdparser_command_t* _cmdparser_find(const char* name)
{
    size_t max, min, guess;
    int cmp;
    min = 0;
    max = _cmdparser_nb - 1;

    while(max - min > 1) {
        guess = (max + min) / 2;
        cmp = strcmp(_cmdparser_cmds[guess].name, name);
        if(cmp == 0)
            return &_cmdparser_cmds[guess];
        else if(cmp < 0)
            min = guess;
        else
            max = guess;
    }

    if(strcmp(_cmdparser_cmds[min].name, name) == 0)
        return &_cmdparser_cmds[min];
    else if(strcmp(_cmdparser_cmds[max].name, name) == 0)
        return &_cmdparser_cmds[max];
    else
        return NULL;
}

bool cmdparser_set_data(const char* name, void* data)
{
    struct _cmdparser_command_t* cmd = _cmdparser_find(name);
    if(!cmd)
        return false;
    cmd->data = data;
    return true;
}

bool cmdparser_change_callback(const char* name,
        cmdparser_callback_t cb, void* data)
{
    struct _cmdparser_command_t* cmd = _cmdparser_find(name);
    if(!cmd)
        return false;
    cmd->cb   = cb;
    cmd->data = data;
    return true;
}

bool cmdparser_parse(const char* line)
{
    char* used = strdup(line);
    char* name;
    struct _cmdparser_command_t* cmd;

    name = strtok(used, " ");
    if(!name) {
        free(used);
        return false;
    }

    cmd = _cmdparser_find(name);
    if(!cmd) {
        free(used);
        return false;
    }

    name = strtok(NULL, "");
    if(name && strlen(name) == 0)
        name = NULL;

    if(cmd->cb)
        cmd->cb(name, cmd->data);
    free(used);
    return true;
}

