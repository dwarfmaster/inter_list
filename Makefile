OBJS=objs/main.o \
	 objs/spawn.o \
	 objs/strformat.o \
	 objs/curses.o \
	 objs/cmdparser.o \
	 objs/events.o \
	 objs/cmdlifo.o \
	 objs/feeder.o \
	 objs/commands.o \
	 objs/bars.o
CFLAGS=-Wall -Wextra -g `pkg-config --cflags ncurses`
LDFLAGS=`pkg-config --libs ncurses`
PROG=list.out
CC=gcc

all : setup $(PROG)

setup :
	mkdir -p objs

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS)    -o $@ $^ $(LDFLAGS)

objs/main.o : src/main.c
	$(CC) $(CFLAGS) -c -o $@ $<

objs/%.o : src/%.c src/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean :
	rm -r objs

rec : clean all

.PHONY:all setup clean rec


