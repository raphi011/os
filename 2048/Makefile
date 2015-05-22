GCC = gcc

TARGET = 2048-server 2048-client
SOURCE = Makefile 2048-server.c 2048-client.c

CFLAGS = -std=c99 -Wall -Wextra -pedantic 

all: $(TARGET)

2048-server: 2048-server.c
	$(GCC) $(CFLAGS) -o $@ $< 

2048-client: 2048-client.c
	$(GCC) $(CFLAGS) -o $@ $< 

clean: 
	rm -f $(TARGET)
