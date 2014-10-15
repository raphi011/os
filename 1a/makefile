CFLAGS= -Wextra -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE

all: stegit.c 
	gcc $(CFLAGS) -o stegit stegit.c

debug: stegit.c
	gcc $(CFLAGS) -g -o stegit stegit.c

clean: 
	rm -f stegit