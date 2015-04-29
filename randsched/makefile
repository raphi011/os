GCC = gcc

CFLAGS = -std=c99 -Wall -Wextra -pedantic 

all: 
	$(GCC) $(CFLAGS) -o rshutdown rshutdown.c
	$(GCC) $(CFLAGS) -o rventgas rventgas.c
	$(GCC) $(CFLAGS) -o schedule schedule.c

clean:
	rm -f rshutdown rventgas schedule
