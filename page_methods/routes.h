#ifndef PAGEMETHODS_INDEX_H_
#define PAGEMETHODS_INDEX_H_

#include <libpq-fe.h> 
#include "../extclib/http.h"

extern void index_page(int conn, HTTPreq *req, PGconn *pg_conn);
extern void about_page(int conn, HTTPreq *req, PGconn *pg_conn);

#endif /* PAGEMETHODS_INDEX_H_ */
