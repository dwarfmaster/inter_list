#!/usr/bin/bash

# Open the list program with a special command.
# Usage : $0 mode [args]
# mode will be searched as a directory in $modes dir (set it below). The mode
# directory must have a list script, which will be used to feed the list.

# Configuration
# The directory with the list of modes
modes="$HOME/.list/modes"
# The path to the list program
list="/usr/bin/list"

# Checking
if [[ $# -lt "1" ]]; then
    echo "Usage : $0 mode [args]" 1>&2
    exit
elif [[ -z $list || ! -x $list ]]; then
    echo "$list can't be executed ..." 1>&2
    exit
elif [[ -z $modes || ! -d $modes ]]; then
    echo "$modes does not exits ..." 1>&2
    exit
fi

# Arguments
path="$modes/$1/list"
if [[ ! -e $path ]]; then
    echo "$1 is not a valid mode ..." 1>&2
    exit
fi

# Executing
exec $list $path ${@:2}
