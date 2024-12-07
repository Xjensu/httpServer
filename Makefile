CC=gcc
CFLAGS=-Wall -std=c99 -I/usr/include/postgresql -I/usr/include/hiredis
LDFLAGS=-lpq -lhiredis

FILES=main.c extclib/extclib.o page_methods/pm.o

.PHONY: default build run clean
default: build run

build: $(FILES)
	$(MAKE) -C page_methods
	$(MAKE) -C extclib
	$(CC) $(CFLAGS) $(FILES) -o main $(LDFLAGS)

run: main
	./main

clean:
	$(MAKE) -C page_methods clean
	$(MAKE) -C extclib clean
	rm -f main
