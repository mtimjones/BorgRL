# Makefile for 7drl 2023 ideas

CC=gcc
CFLAGS=-std=c18 -Wall -Werror -Wextra -g
LIBS= -lncurses -lm

DEPS = headers.h ces.h logapi.h behaviors.h stats.h
OBJS = main.o screen.o time.o messages.o user_input.o game.o map.o select.o ces.o \
	levels.o astar.o logapi.o behaviors.o stats.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

demo: $(OBJS)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f demo *.o log.txt
