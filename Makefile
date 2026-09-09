CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

all: scheduler

scheduler: scheduler.c
	$(CC) $(CFLAGS) scheduler.c -o scheduler

clean:
	rm -f scheduler

.PHONY: all clean