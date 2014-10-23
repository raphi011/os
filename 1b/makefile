# File:		stegit
# Author:	Raphael Gruber ( 0828630 )
# Date:  	17.10.2014         


CFLAGS= -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -Wall
FILES= client.c server.c

all: $(FILES)
	gcc $(CFLAGS) -o server server.c
	gcc $(CFLAGS) -o client client.c

debug: $(FILES)
	gcc $(CFLAGS) -g -o server server.c
	gcc $(CFLAGS) -g -o client client.c

clean: 
	rm -f server client
