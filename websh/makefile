CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: websh

websh: websh.o 
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

websh.o: websh.c

clean:
	rm -f websh.o websh