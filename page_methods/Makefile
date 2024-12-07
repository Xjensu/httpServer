CC=gcc
LN=ld
CFLAGS=-Wall -std=c99 -I/usr/include/postgresql -I/usr/include/hiredis

HEADERS = routes.h 
SOURCES = routes.c
OBJECTS = routes.o

.PHONY: default clean
default: build link

build: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)

link: $(OBJECTS)
	$(LN) -r $(OBJECTS) -o pm.o
    
clean:
	rm -f *.o *.i *.s
