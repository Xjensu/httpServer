#include <stdio.h> 
#include <stdio.h> 
#include <string.h> 
#include <libpq-fe.h> 
#include <hiredis/hiredis.h>

#include "extclib/http.h" 
#include "page_methods/routes.h"
#include "config.h"

int main(void) { 
	char conninfo[256];
	snprintf(conninfo, 256, "user=%s dbname=%s host=%s password=%s", 
		POSTGRES_USER, POSTGRES_DB, POSTGRES_HOST, POSTGRES_PASSWORD); 
	PGconn *pg_conn = PQconnectdb(conninfo); 
	if (PQstatus(pg_conn) != CONNECTION_OK) { 
		fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(pg_conn)); 
		PQfinish(pg_conn); 
		return 1; 
	}
	HTTP *serve = new_http("127.0.0.1:7545", pg_conn); 

	handle_http(serve, "/", (void (*)(int, HTTPreq *, PGconn *)) index_page); 
	handle_http(serve, "/about/", (void (*)(int, HTTPreq *, PGconn *))about_page);
	handle_http(serve, "/register", (void (*)(int, HTTPreq *, PGconn *))register_page);
 
	listen_http(serve);

	free_http(serve); 
	PQfinish(pg_conn);
	return 0; 
}

