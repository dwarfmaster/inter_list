
#include "feeder.h"
#include "spawn.h"
#include "curses.h"

/* The process of the feeder. */
static spawn_t _feeder_sp;
/* A read line. */
struct _feeder_line_t {
    /* The text of the line. */
    char* line;
    /* The id of the line. */
    char* id;
};
/* The array of all the line read. */
static struct _feeder_line_t*  _feeder_lines;
static size_t                  _feeder_nb;
static size_t                  _feeder_capa;

bool feeder_init()
{
    _feeder_nb     = 0;
    _feeder_capa   = 50;
    _feeder_lines  = malloc(sizeof(struct _feeder_line_t) * _feeder_capa);
    _feeder_sp     = spawn_init();
    return _feeder_lines;
}

void feeder_quit()
{
    size_t i;
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
    size_t i;
    spawn_close(&_feeder_sp);
    if(_feeder_lines) {
        for(i = 0; i < _feeder_nb; ++i) {
            free(_feeder_lines[i].id);
            free(_feeder_lines[i].line);
        }
    }
    _feeder_nb = 0;
    curses_list_changed(false);

    _feeder_sp = spawn_create_shell(command);
    return spawn_ok(_feeder_sp);
}

int feeder_fd()
{
    if(spawn_ok(_feeder_sp))
        return spawn_fd(_feeder_sp);
    else
        return -1;
}

/* Add a new read line to the array. */
static void _feeder_add_line(char* line)
{
    struct _feeder_line_t ln;
    char* strtokbuf;

    ln.id   = strtok_r(line, "\t", &strtokbuf);
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
    ++_feeder_nb;
}

void feeder_update()
{
    char buffer[4096];
    size_t cont;
    char* line;
    char* strtokbuf;

    if(!spawn_ok(_feeder_sp))
        return;

    if((cont = spawn_read(_feeder_sp, buffer, 4095)) != 0) {
        buffer[cont] = '\0';

        line = strtok_r(buffer, "\n", &strtokbuf);
        while(line) {
            _feeder_add_line(line);
            line = strtok_r(NULL, "\n", &strtokbuf);
        }
    }
}

size_t feeder_get_id()
{
    return curses_list_get();
}

size_t feeder_get_lines()
{
    return _feeder_nb;
}

const char* feeder_get_name(size_t id)
{
    if(id >= _feeder_nb)
        return NULL;
    else
        return _feeder_lines[id].id;
}

const char* feeder_get_text(size_t id)
{
    if(id >= _feeder_nb)
        return NULL;
    else
        return _feeder_lines[id].line;
}

feeder_iterator_t feeder_begin()
{
    feeder_iterator_t it;
    it.id    = 0;
    it.vid   = it.id;
    it.valid = (_feeder_nb != 0);
    return it;
}

feeder_iterator_t feeder_end()
{
    feeder_iterator_t it;
    it.id    = _feeder_nb;
    it.vid   = it.id;
    it.valid = false;
    return it;
}

feeder_iterator_t feeder_next(feeder_iterator_t* it, size_t n)
{
    size_t count;

    if(!it->valid)
        return *it;

    count = 0;
    while(it->id++ < _feeder_nb && count++ < n);
    if(count != n)
        it->valid = false;
    it->vid = it->id;
    return *it;
}

feeder_iterator_t feeder_prev(feeder_iterator_t* it, size_t n)
{
    size_t count;

    if(!it->valid)
        return *it;

    count = 0;
    while(it->id-- > 1 && count++ < n);
    if(it->id == 0 && count < n-1)
        it->valid = false;
    it->vid = it->id;
    return *it;
}

const char* feeder_get_it_text(feeder_iterator_t it)
{
    if(!it.valid)
        return NULL;
    return _feeder_lines[it.id].line;
}

const char* feeder_get_it_name(feeder_iterator_t it)
{
    if(!it.valid)
        return NULL;
    return _feeder_lines[it.id].id;
}

int feeder_it_cmp(feeder_iterator_t it1, feeder_iterator_t it2)
{
    return it1.id - it2.id;
}

