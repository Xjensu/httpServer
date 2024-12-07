CC=gcc
CFLAGS=-Wall -std=c99 -I/usr/include/postgresql -I/usr/include/hiredis
LDFLAGS=-lpq -lhiredis

FILES=main.c extclib/extclib.o

.PHONY: default build run
default: build run

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o main $(LDFLAGS)

run: main
	./main
