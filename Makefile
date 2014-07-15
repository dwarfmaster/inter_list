OBJS=objs/main.o \
	 objs/spawn.o \
	 objs/strformat.o
CFLAGS=-Wall -Wextra -g
LDFLAGS=
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


