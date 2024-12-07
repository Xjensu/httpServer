CC=gcc
LN=ld
CFLAGS=-Wall -std=c99 -I/usr/include/postgresql -I/usr/include/hiredis

HEADERS = cJSON.h http.h net.h hashtab.h tree.h type.h
SOURCES = cJSON.c http.c net.c hashtab.c tree.c type.c
OBJECTS = cJSON.o http.o net.o hashtab.o tree.o type.o

.PHONY: default
default: build link

build: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)
link: $(OBJECTS)
	$(LN) -r $(OBJECTS) -o extclib.o
	
clean:
	rm -f *.0 *.i *.s

