# NCurses List Selecter
This software aims to create a ncurses list-based interface out of any set of
cli tools.

## Interface.
The interface includes four parts. On the top of the screen is a status bar,
also refered to as the status bar. This line can be disabled. On the bottom of
the screen they are two lines. A bottom status bar, and a command line. The
bottom bar can also be disabled, while the command line cannot. The command
line is used to get textual input from the user. The rest of the screen is the
list.

## Basic usage.
When launched, this program will set up a 9P server, which must be used to
interact with. This program takes only one argument (but it is mandatory) : the
path to the socket to create for the 9P connection. For the moment, only unix
sockets are handled.

## FS organisation.
All these files and directory are used to interact with the program.
 - `ctl` : basic file to handle commands. This file is write-only and only
     supports append writes. See ctl commands parts for the list of accepted
     commands.
 - `feed` : set the feeding program. Any line written to it will be considered
     to be the feeding program. Empty lines are ignored. Writing another line
     will clear the list (you can't append the output of one feed to another).
     When read, this file give the feeding program as it was executed.
 - `bindings` : bind one event to an action. The event may either be an
     internal event or a key combinaison. See the bindings part for details.
     Empty lines are ignored. When read, this files gives the mapped events.
 - `top` : allow interacting with the top bar. Any line written to it will be
     displayed in the top bar. Writting another line will discard the actual
     content. Writting and empty line will discard the top bar. When read, it
     gives the content of the top bar.
 - `bot` : same as `top` but for the bottom bar.
 - `colors` : set the foreground and background colors of each part of the
     interface. See the colors part for details. This is a read-only file.
 - `list` : this directory contains all the detail about the contents of the
     list.
   - `scroll` : set the scroll mode. Written values must be either `pager` or
       `list`. If the scroll mode is `pager`, the list elements will be
       displayed page by page : when reaching the bottom of the screen, the
       selection will be placed on the top of the screen and new lines will be
       displayed, while in `list` mode, there will be a simple scrolling. When
       read, this file gives the actual mode.
   - `selection` : set the selected line. It can either be an absolute value
       or a relative one, like `+4` or `-1`. Finally, it also accepts two
       special commands : `begin` and `end`. If setting an absolute value out
       of range, the selection won't be changed. If using a relative value
       leading out of range, it will stop either on the first or last value.
       When read, it gives the absolute id of the selected line.
   - //id// : directories which name is the id of the line. They each represent
       a line.
     - `text` : Read-only file giving the text of the line.
     - `name` : Read-only file giving the name of the line (see the feeding
         part for the meaning of this value).
     - `show` : boolean file indicating wether the line is shown or not. By
        default, all the lines are shown. Values accepted are `0` and `1`.
        Changes won't be taken immediately into account : the refresh command
        must be sent to the `/ctl` file before. When read, it indicates if the
        line is shown or not.

## CTL commands.
Here is the list of the commands accepted :
 - `quit` : close the program.
 - `refresh` : update the screen. It is necessary to send it after hiding or
     showing lines.
 - `term mode` : `mode` must be either `on`, to enable the term-escape, `off`
     to disable it or `toggle` to toggle it.

## Bindings.
TODO

## Colors.
They are eight supported colors, which are designated by their name :
 - `black`
 - `red`
 - `green`
 - `yellow`
 - `blue`
 - `magenta`
 - `cyan`
 - `white`

## Feeding
The feeding is the act of populating the list. It is done by the program
written to the `/feed` file. The feeding program must output a list of entries,
one in each line of its output. Each entry must include two parts, separated by
a tabulation. The first part, which won't be displayed in the list, is the name
of the entry. It cannot contain tabulations. It is used to refer to the entryon
the list. The second part is the string displayed in the list. It can contains
tabulations.

No sorting of any kind will be done : the entries will be displayed in the
order they are outputed by the feeding program.

Writting to the `/feed` file while there already was a feeding program setted
will clear the list before setting the new feeding program.

## Examples
The examples are here to show how to write scripts to use this program. For the
moment, there is only one. To execute it, you must launch the program with the
path `examples/files/list.pl`, and then the directory you want to browse. The
content of the directory will be displayed. You can move around with the arroy
key and the `return` key to open a file. If the file is a directory, its
content will be displayed. If it is a regular file, it will be opened with vim.
You can change the action when opening a file by editing the
`examples/files/open.pl` script.

## TODO-list
 - [ ] write a complete documentation.
 - [X] let the user configure colors.
 - [X] handle UTF-8.
 - [ ] handle mouse.
 - [X] handle hiding.
 - [ ] handle combinaisons keybindings.
 - [ ] handle 9P protocol.

