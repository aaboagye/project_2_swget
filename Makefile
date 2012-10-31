CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -D_POSIX_SOURCE -g

all:	swget

swget:	swget.c swget.h
	$(CC) $(CFLAGS) -o swget swget.c

clean:
	rm -f swget
