CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -D_POSIX_SOURCE -g

all:	swget

swget:	swget.c
	$(CC) $(CFLAGS) -o swget swget.c

clean: swget
	rm swget
