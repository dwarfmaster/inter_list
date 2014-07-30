
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
    _bars_top = strformat_parse(_bars_symbs, br);
    if(!_bars_top)
        return false;
    bars_update();
    return true;
}

bool bars_bot_set(const char* br)
{
    if(_bars_bot)
        strformat_destroy(_bars_bot);
    _bars_bot = strformat_parse(_bars_symbs, br);
    if(!_bars_bot)
        return false;
    bars_update();
    return true;
}

void bars_update()
{
    size_t i, nb;
    char buffer[256];

    i = feeder_get_id();
    snprintf(buffer, 256, "%li", i);
    strformat_set(_bars_symbs, 'i', buffer);

    nb = feeder_get_lines();
    snprintf(buffer, 256, "%li", nb);
    strformat_set(_bars_symbs, 'I', buffer);

    strformat_set(_bars_symbs, 'n', feeder_get_name(i));
    strformat_set(_bars_symbs, 't', feeder_get_text(i));

    curses_top_set(strformat_get(_bars_top));
    curses_bot_set(strformat_get(_bars_bot));
}

