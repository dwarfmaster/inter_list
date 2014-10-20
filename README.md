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
When invoking this program, you must give it the path to a program via the
command line. The stdout of the program will be read and interpreted as a set
of commands (see next paragraph for available commands). The commands must be
separated by newlines.

## Available commands.
 - `up    [nb]`  : move the selection up nb lines. nb defaults to 1.
 - `down  [nb]`  : move the selection down nb lines. nb defaults to 1.
 - `right [nb]`  : move the lines right nb characters. nb defaults to 1.
 - `left  [nb]`  : move the lines left nb characters. nb defaults to 1.
 - `begin`       : move the selection to the first line.
 - `end`         : move the selection to the last line.
 - `goto  [nb]`  : move the selection to le nb-eme line. If nb is out of range,
                 the selection won't be moved.
 - `scroll mode` : set the scroll mode. mode must be either `pager`, `list` or
                   `toggle`. If the scroll mode is `pager`, the list elements
                   will be displayed page by page : when reaching the bottom of
                   the screen, the selection will be placed on the top of the
                   screen and new lines will be displayed, while in `list`
                   mode, there will be a simple scrolling. `toggle` simply
                   changes the actual scroll mode to the other one.
 - `quit`        : end the program.
 - `exe str`     : str will be parsed as a command.
 - `map key cmd` : cmd will be executed when key combinaison is pressed. See
                 keybinds paragraph for details of the syntax of key.
 - `feed prog`   : prog must the path to a program which will be spawned. Its
                 stdout will be used to populate the list contents. See the
                 feeding paragraph for details on how its output must be
                 formatted.
 - `spawn prog`  : will spawn prog and read its output as a set of commands.
 - `term prog`   : prog will be spawned in a shell escape. It's stdout will be
                 displayed to the used.
 - `refresh`     : redraw the screen.
 - `top [str]`   : set the contents of the top bar. If there is not str, the
                   top bar will be disabled. There are symbols which will
                   replaced by values : `%i` will be replaced by the index
                   of the selected entry, `%I` will be replaced by the number
                   of entries, `%n` will be replaced by the name of the
                   selected entry and `%t` by its text.
 - `bot [str]`   : work the same as the top command, but for the bottom bar.
 - `color [part] [fg] [bg]` : define the background and foreground colors of a
                            part of the interface. part can be either `top`,
                            `bot`, `lst` or `sel`. `fg` and `bg` are colors,
                            so they can be the name of any of the eight colors
                            supported by ncurses.

## Keybinds.
TODO

## Feeding
The feeding is the act of populating the list. It is done by a program setted
by the `feed` command. The feeding program must output a list of entries, one
in each line of its output. Each entry must include two parts, separated by a
tabulation. The first part, which won't be displayed in the list, is the name
of the entry. It cannot contain tabulations. It is used to refer to the entry
on the list. The second part is the string displayed in the list. It can
contains tabulations.

No sorting of any sort will be done : the entries will be displayed in the
order they are outputed by the feeding program.

Using the feed command while there already was a feeding program setted will
clear the list before setting the new feeding program.

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
 - [ ] handle hiding.
 - [ ] handle combinaisons keybindings.

