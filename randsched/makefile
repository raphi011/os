# File:		randsched
# Author: 	Raphael Gruber (0828630)
# Date:		02.05.2015

GCC = gcc

CFLAGS = -std=c99 -Wall -Wextra -pedantic -D_XOPEN_SOURCE=500 -D_BSD_SOURCE

all: 
	$(GCC) $(CFLAGS) -o rshutdown rshutdown.c
	$(GCC) $(CFLAGS) -o rventgas rventgas.c
	$(GCC) $(CFLAGS) -o schedule schedule.c

debug: 
	$(GCC) $(CFLAGS) -g -o rshutdown rshutdown.c
	$(GCC) $(CFLAGS) -g -o rventgas rventgas.c
	$(GCC) $(CFLAGS) -g -o schedule schedule.c

clean:
	rm -f rshutdown rventgas schedule
