#ifndef EXTCLIB_HTTP_H_
#define EXTCLIB_HTTP_H_

#include <stdint.h>
#include <libpq-fe.h>

typedef struct HTTPreq {
    char method[16]; // GET
    char path[2048]; // /books
    char proto[16];  // HTTP/1.1
    char body[BUFSIZ]; // Добавляем поле для тела запроса
    uint8_t state;
    size_t index;
} HTTPreq;


typedef struct HTTP HTTP;

extern HTTP *new_http(char *address, PGconn *pg_conn);
extern void free_http(HTTP *http);

void handle_http(HTTP *http, char *path, void(*handle)(int, HTTPreq*, PGconn*));
extern int8_t listen_http(HTTP *http);

extern void parsehtml_http(int conn, char *filename);

extern void _page404_http(int conn);

#endif /* EXTCLIB_HTTP_H_ */
