#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <hiredis/hiredis.h>

#include "../extclib/http.h"
#include "../extclib/net.h"

extern void index_page(int conn, HTTPreq *req, PGconn *pg_conn) {
    printf("Handling index page for path: %s\n", req->path);
    printf("%s - %s - %s\n", req->method, req->path, req->proto);
    
    if (strcmp(req->path, "/") != 0) {
        parsehtml_http(conn, "page404.html");
        return;
    }

    printf("Checking database connection status...\n");
    if (PQstatus(pg_conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection is not OK: %s\n", PQerrorMessage(pg_conn));
        parsehtml_http(conn, "page404.html");
        return;
    }

    printf("Database connection is active.\n");
    printf("Executing simple query: SELECT 1\n");
    PGresult *res = PQexec(pg_conn, "SELECT 1");
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute simple query: %s\n", PQerrorMessage(pg_conn));
        PQclear(res);
        parsehtml_http(conn, "page404.html");
        return;
    }

    printf("Simple query executed successfully.\n");
    PQclear(res);

    // Выполняем запрос к таблице пользователей
    printf("Executing query: SELECT * FROM users\n");
    PGresult *user_res = PQexec(pg_conn, "SELECT * FROM users");
    
    if (PQresultStatus(user_res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute user query: %s\n", PQerrorMessage(pg_conn));
        PQclear(user_res);
        parsehtml_http(conn, "page404.html");
        return;
    }

    if (PQntuples(user_res) == 0) {
        printf("No data found in the users table.\n");
        char response[BUFSIZ];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>No users found</h1></body></html>");
        send_net(conn, response, strlen(response));
        PQclear(user_res);
        return;
    }

    printf("User query executed successfully, processing results.\n");

    char buffer[BUFSIZ];
    snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Users</h1><ul>");

    for (int i = 0; i < PQntuples(user_res); i++) {
        char user_entry[256];
        snprintf(user_entry, sizeof(user_entry), "<li>%s (%s)</li>", PQgetvalue(user_res, i, 1), PQgetvalue(user_res, i, 2));
        strncat(buffer, user_entry, sizeof(buffer) - strlen(buffer) - 1);
    }

    strncat(buffer, "</ul></body></html>", sizeof(buffer) - strlen(buffer) - 1);
    printf("Sending response.\n");
    send_net(conn, buffer, strlen(buffer));
    printf("Response sent.\n");

    PQclear(user_res);
    printf("Cleared result sets.\n");
}



extern void about_page(int conn, HTTPreq *req, PGconn *pg_conn) {
    printf("Handling about page for path: %s\n", req->path);

    if (strncmp(req->path, "/about/user?=", 13) != 0) {
        parsehtml_http(conn, "page404.html");
        return;
    }

    // Получаем ID пользователя из пути
    int user_id = atoi(req->path + 13);

    // Выполняем запрос к базе данных для получения информации о пользователе
    char query[256];
    snprintf(query, 256, "SELECT * FROM users WHERE id = %d", user_id);
    PGresult *res = PQexec(pg_conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", PQerrorMessage(pg_conn));
        PQclear(res);
        parsehtml_http(conn, "page404.html");
        return;
    }

    if (PQntuples(res) == 0) {
        // Если данные не найдены, выводим сообщение об этом
        printf("No data found for user with ID %d.\n", user_id);
        char response[BUFSIZ];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>No user found with ID %d</h1></body></html>", user_id);
        send_net(conn, response, strlen(response));
        PQclear(res);
        return;
    }

    // Формируем HTML-ответ
    char buffer[BUFSIZ];
    snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>User Info</h1>");
    strncat(buffer, "<p>Login: ", sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, PQgetvalue(res, 0, 1), sizeof(buffer) - strlen(buffer) - 1); // login
    strncat(buffer, "</p><p>Email: ", sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, PQgetvalue(res, 0, 2), sizeof(buffer) - strlen(buffer) - 1); // email
    strncat(buffer, "</p><p>Date of Register: ", sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, PQgetvalue(res, 0, 4), sizeof(buffer) - strlen(buffer) - 1); // date_of_register
    strncat(buffer, "</p></body></html>", sizeof(buffer) - strlen(buffer) - 1);

    printf("Sending response.\n");
    send_net(conn, buffer, strlen(buffer));
    printf("Response sent.\n");

    PQclear(res); // Очищаем результат запроса
}



