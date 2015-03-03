# File:		stegit
# Author:	Raphael Gruber ( 0828630 )
# Date:  	17.10.2014         


CFLAGS= -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wall

all: stegit.c
	gcc $(CFLAGS) -o stegit stegit.c

debug: stegit.c
	gcc $(CFLAGS) -g -o stegit stegit.c

clean: 
	rm -f stegit
