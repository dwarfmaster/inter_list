
#include "feeder.h"
#include "spawn.h"
#include "curses.h"
#include <stdlib.h>

static spawn_t _feeder_sp;
static bool    _feeder_setted;

struct _feeder_line_t {
    char* line;
    char* id;
};
static struct _feeder_line_t*  _feeder_lines;
static size_t                  _feeder_nb;
static size_t                  _feeder_capa;

bool feeder_init()
{
    _feeder_setted = false;
    _feeder_nb     = 0;
    _feeder_capa   = 50;
    _feeder_lines  = malloc(sizeof(struct _feeder_line_t) * _feeder_capa);
    return _feeder_lines;
}

void feeder_quit()
{
    size_t i;
    if(_feeder_setted)
        spawn_close(&_feeder_sp);
    if(_feeder_lines) {
        for(i = 0; i < _feeder_nb; ++i) {
            free(_feeder_lines[i].id);
            free(_feeder_lines[i].line);
        }
        free(_feeder_lines);
    }
}

bool feeder_set(const char* command)
{
    if(_feeder_setted)
        spawn_close(&_feeder_sp);

    _feeder_sp = spawn_create_shell(command);
    curses_list_clear();
    return (_feeder_setted = spawn_ok(_feeder_sp));
}

int feeder_fd()
{
    if(_feeder_setted)
        return spawn_fd(_feeder_sp);
    else
        return -1;
}

static void _feeder_add_line(char* line)
{
    struct _feeder_line_t ln;
    char* strtokbuf;
    const char* lines[1];

    ln.id   = strtok_r(line, " ", &strtokbuf);
    ln.line = strtok_r(NULL, "",  &strtokbuf);
    if(!ln.id || !ln.line)
        return;
    ln.id   = strdup(ln.id);
    ln.line = strdup(ln.line);

    if(_feeder_nb >= _feeder_capa) {
        _feeder_capa += 50;
        _feeder_lines = realloc(_feeder_lines,
                sizeof(struct _feeder_line_t) * _feeder_capa);
        if(!_feeder_lines) {
            free(ln.id);
            free(ln.line);
            return;
        }
    }

    _feeder_lines[_feeder_nb] = ln;
    lines[0] = ln.line;
    curses_list_add_lines(1, lines);
    ++_feeder_nb;
}

void feeder_update()
{
    char buffer[4096];
    size_t cont;
    char* line;
    char* strtokbuf;

    if(!_feeder_setted)
        return;

    while((cont = spawn_read(_feeder_sp, buffer, 4095)) != 0) {
        buffer[cont] = '\0';

        line = strtok_r(buffer, "\n", &strtokbuf);
        while(line) {
            _feeder_add_line(line);
            line = strtok_r(NULL, "\n", &strtokbuf);
        }
    }
}

