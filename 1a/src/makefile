CFLAGS= -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wall

all: stegit.c 
	gcc-4.9 $(CFLAGS) -o stegit stegit.c

debug: stegit.c
	gcc-4.9 $(CFLAGS) -g -o stegit stegit.c

clean: 
	rm -f stegit