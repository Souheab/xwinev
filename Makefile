CC=gcc
CFLAGS=-o xwinev xwinev.c -lX11 -Wall -Wextra

all: xwinev

xwinev: xwinev.c
	$(CC) $(CFLAGS)

debug: CFLAGS += -DDEBUG -g
debug: xwinev

clean:
	rm -f xwinev
