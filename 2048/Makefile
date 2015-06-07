GCC = gcc

TARGET = 2048-server 2048-client
SOURCE = Makefile 2048-server.c 2048-client.c

CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -g
LFLAGS = -lrt -pthread -lm

all: $(TARGET)

2048-server: 2048-server.c helper.c
	$(GCC) $(CFLAGS) -o $@ $^ $(LFLAGS)

2048-client: 2048-client.c helper.c
	$(GCC) $(CFLAGS) -o $@ $^ $(LFLAGS)

clean: 
	rm -f $(TARGET)
