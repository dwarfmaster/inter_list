
#include "bars.h"
#include "strformat.h"
#include "feeder.h"
#include "curses.h"
#include <stdio.h>

static strformat_symbs_t* _bars_symbs;
static strformat_t*       _bars_top;
static strformat_t*       _bars_bot;

bool bars_init()
{
    _bars_top = NULL;
    _bars_bot = NULL;
    _bars_symbs = strformat_symbols("ntiI");
    return _bars_symbs;
}

void bars_quit()
{
    if(_bars_symbs)
        strformat_symbols_destroy(_bars_symbs);
    if(_bars_top)
        strformat_destroy(_bars_top);
    if(_bars_bot)
        strformat_destroy(_bars_bot);
}

bool bars_top_set(const char* br)
{
    if(_bars_top)
        strformat_destroy(_bars_top);

    if(!br)
        _bars_top = NULL;
    else {
        _bars_top = strformat_parse(_bars_symbs, br);
        if(!_bars_top)
            return false;
    }
    bars_update();
    return true;
}

bool bars_bot_set(const char* br)
{
    if(_bars_bot)
        strformat_destroy(_bars_bot);

    if(!br)
        _bars_bot = NULL;
    else {
        _bars_bot = strformat_parse(_bars_symbs, br);
        if(!_bars_bot)
            return false;
    }
    bars_update();
    return true;
}

void bars_update()
{
    size_t i;
    feeder_iterator_t it;
    char buffer[256];

    i = curses_list_get();
    snprintf(buffer, 256, "%lu", i + 1);
    strformat_set(_bars_symbs, 'i', buffer);

    it = feeder_end();
    snprintf(buffer, 256, "%lu", it.id);
    strformat_set(_bars_symbs, 'I', buffer);

    it = feeder_begin();
    feeder_next(&it, i);
    strformat_set(_bars_symbs, 'n', feeder_get_it_name(it));
    strformat_set(_bars_symbs, 't', feeder_get_it_text(it));

    if(_bars_top)
        curses_top_set(strformat_get(_bars_top));
    else
        curses_top_set(NULL);

    if(_bars_bot)
        curses_bot_set(strformat_get(_bars_bot));
    else
        curses_top_set(NULL);
}

