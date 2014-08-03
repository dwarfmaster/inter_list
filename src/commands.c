
#include "commands.h"
#include "spawn.h"
#include "strformat.h"
#include "curses.h"
#include "cmdparser.h"
#include "events.h"
#include "cmdlifo.h"
#include "feeder.h"
#include "bars.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

static void _commands_up(const char* str, void* data)
{
    size_t up = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%lu", &up);
    curses_list_up(up);
}

static void _commands_down(const char* str, void* data)
{
    size_t down = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%lu", &down);
    curses_list_down(down);
}

static void _commands_left(const char* str, void* data)
{
    size_t left = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%lu", &left);
    curses_list_left(left);
}

static void _commands_right(const char* str, void* data)
{
    size_t right = 1;
    if(data) { } /* avoid warnings */
    if(str)
        sscanf(str, "%lu", &right);
    curses_list_right(right);
}

static void _commands_begin(const char* str, void* data)
{
    if(data && str) { } /* avoid warnings */
    curses_list_set(0);
}

static void _commands_end(const char* str, void* data)
{
    if(data && str) { } /* avoid warnings */
    curses_list_set(feeder_get_lines() - 1);
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

static void _commands_term(const char* str, void* data)
{
    if(data) { } /* avoid warnings. */
    if(!str)
        return;

    curses_disable();
    spawn_exec_shell(str);
    curses_enable();
}

static void _commands_refresh(const char* str, void* data)
{
    if(data && str) { } /* avoid warnings */
    curses_redraw();
}

static void _commands_top(const char* str, void* data)
{
    if(data) { } /* avoid warnings */
    bars_top_set(str);
}

static void _commands_bot(const char* str, void* data)
{
    if(data) { } /* avoid warnings */
    bars_bot_set(str);
}

static void _commands_color(const char* str, void* data)
{
    char* strtokbuf;
    char* part;
    char* fg;
    char* bg;
    char* used;
    if(data) { } /* avoid warnings */

    used = strdup(str);
    part = strtok_r(used, " ", &strtokbuf);
    fg   = strtok_r(NULL, " ", &strtokbuf);
    bg   = strtok_r(NULL, " ", &strtokbuf);
    if(!part || !fg || !bg) {
        free(used);
        return;
    }

    if(strcmp(part, "top") == 0)
        curses_top_colors(curses_str_to_color(fg), curses_str_to_color(bg));
    else if(strcmp(part, "bot") == 0)
        curses_bot_colors(curses_str_to_color(fg), curses_str_to_color(bg));
    else if(strcmp(part, "lst") == 0)
        curses_list_colors(curses_str_to_color(fg), curses_str_to_color(bg));
    else if(strcmp(part, "sel") == 0)
        curses_list_colors_sel(curses_str_to_color(fg),
                curses_str_to_color(bg));
    free(used);
}

void commands_setup(bool* cont)
{
    cmdparser_add_command("up",      &_commands_up,      NULL);
    cmdparser_add_command("down",    &_commands_down,    NULL);
    cmdparser_add_command("right",   &_commands_right,   NULL);
    cmdparser_add_command("left",    &_commands_left,    NULL);
    cmdparser_add_command("begin",   &_commands_begin,   NULL);
    cmdparser_add_command("end",     &_commands_end,     NULL);

    cmdparser_add_command("quit",    &_commands_quit,    cont);
    cmdparser_add_command("exe",     &_commands_exe,     NULL);
    cmdparser_add_command("map",     &_commands_map,     NULL);
    cmdparser_add_command("feed",    &_commands_feed,    NULL);
    cmdparser_add_command("spawn",   &_commands_spawn,   NULL);
    cmdparser_add_command("term",    &_commands_term,    NULL);

    cmdparser_add_command("refresh", &_commands_refresh, NULL);
    cmdparser_add_command("top",     &_commands_top,     NULL);
    cmdparser_add_command("bot",     &_commands_bot,     NULL);
    cmdparser_add_command("color",   &_commands_color,   NULL);
}

