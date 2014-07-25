
#include "commands.h"
#include "spawn.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include "events.h"
#include "cmdlifo.h"
#include "feeder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void _commands_up(const char* str, void* data)
{
    unsigned int up = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &up);
    curses_list_up(up);
}

static void _commands_down(const char* str, void* data)
{
    unsigned int down = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &down);
    curses_list_down(down);
}

static void _commands_left(const char* str, void* data)
{
    unsigned int left = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &left);
    curses_list_left(left);
}

static void _commands_right(const char* str, void* data)
{
    unsigned int right = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%u", &right);
    curses_list_right(right);
}

static void _commands_quit(const char* str, void* data)
{
    if(str) { } /* avoid warnings */
    bool* cont = data;
    *cont = false;
}

static void _commands_exe(const char* str, void* data)
{
    if(data) { } /* avoid warnings */
    if(str)
        cmdparser_parse(str);
}

static void _commands_map(const char* str, void* data)
{
    if(data) { } /* avoid warnings */
    if(!str)
        return;

    char* keys = NULL;
    char* action = NULL;
    char* used = strdup(str);
    size_t i;
    size_t nb;

    nb = 0;
    for(i = 0; i < strlen(used); ++i) {
        if(used[i] == '<')
            ++nb;
        else if(nb > 0 && used[i] == '>')
            --nb;
        else if(nb == 0 && used[i] == ' ') {
            used[i] = '\0';
            keys   = used;
            action = used + i + 1;
            break;
        }
    }

    if(keys && strlen(keys) != 0 
            && action && strlen(action) != 0)
        events_add(keys, action);
    free(used);
}

static void _commands_feed(const char* str, void* data)
{
    if(data) { } /* avoid warnings. */
    if(!str)
        return;
    feeder_set(str);
}

static void _commands_spawn(const char* str, void* data)
{
    if(!data) { } /* avoid warnings */
    if(!str)
        return;
    cmdlifo_push(str);
}

static void _commands_refresh(const char* str, void* data)
{
    if(data && str) { } /* avoid warnings */
    curses_redraw();
}

void commands_setup(bool* cont)
{
    cmdparser_add_command("up",      &_commands_up,      NULL);
    cmdparser_add_command("down",    &_commands_down,    NULL);
    cmdparser_add_command("right",   &_commands_right,   NULL);
    cmdparser_add_command("left",    &_commands_left,    NULL);
    cmdparser_add_command("quit",    &_commands_quit,    cont);
    cmdparser_add_command("exe",     &_commands_exe,     NULL);
    cmdparser_add_command("map",     &_commands_map,     NULL);
    cmdparser_add_command("feed",    &_commands_feed,    NULL);
    cmdparser_add_command("spawn",   &_commands_spawn,   NULL);
    cmdparser_add_command("refresh", &_commands_refresh, NULL);
}

