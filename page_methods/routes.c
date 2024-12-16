#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <hiredis/hiredis.h>

#include "../extclib/http.h"
#include "../extclib/net.h"
#include "../extclib/cJSON.h"

#include "secure.h"

cJSON* _create_user_json(char *status, char *login, char *email, char *date_of_register);
char* _create_users_json_response(PGresult *res); 
char* _create_error_json();

extern void index_page(int conn, HTTPreq *req, PGconn *pg_conn) {
    printf("%s - %s - %s\n", req->method, req->path, req->proto);
    
    if (strcmp(req->path, "/") != 0) {
        _page404_http(conn);
        return;
    }

    if (PQstatus(pg_conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection is not OK: %s\n", PQerrorMessage(pg_conn));
        _page404_http(conn);
        return;
    }

    PGresult *user_res = PQexec(pg_conn, "SELECT * FROM users");
    
    if (PQresultStatus(user_res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute user query: %s\n", PQerrorMessage(pg_conn));
        PQclear(user_res);
        _page404_http(conn);
        return;
    }

    if (PQntuples(user_res) == 0) {
        char *json_response = _create_error_json("404", "No users found");
        char response[BUFSIZ];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response);
        send_net(conn, response, strlen(response));
        free(json_response);
        PQclear(user_res);
        return;
    }

    char *json_response = _create_users_json_response(user_res);
    char buffer[BUFSIZ];
    snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response);
    send_net(conn, buffer, strlen(buffer)); 

    free(json_response);
    PQclear(user_res);
}



extern void about_page(int conn, HTTPreq *req, PGconn *pg_conn) {
    if (strncmp(req->path, "/about/user?=", 13) != 0) {
        _page404_http(conn);
        return;
    }

    int user_id = atoi(req->path + 13);

    char query[256];
    snprintf(query, 256, "SELECT * FROM users WHERE id = %d", user_id);
    PGresult *res = PQexec(pg_conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", PQerrorMessage(pg_conn));
        PQclear(res);
        _page404_http(conn);
        return;
    }

    if (PQntuples(res) == 0) {
        char *json_response = _create_error_json("404", "No user found with given ID");
        char response[BUFSIZ];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response);
        send_net(conn, response, strlen(response));
        free(json_response);
        PQclear(res);
        return;
    }

    cJSON *user = _create_user_json("200", PQgetvalue(res, 0,1), PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 4));
    char *json_response = cJSON_Print(user);
    cJSON_Delete(user);
    char buffer[BUFSIZ];
    snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response);
    send_net(conn, buffer, strlen(buffer));

    free(json_response);
    PQclear(res);
}

extern void register_page(int conn, HTTPreq *req, PGconn *pg_conn) { 
	if (strcmp(req->method, "POST") != 0) { 
		_page404_http(conn); 
		return; 
	}

	printf("Received body: %s\n", req->body);
	cJSON *json = cJSON_Parse(req->body); 
	if (!json) { 
		printf("Failed to parse JSON. Received body: %s\n", req->body);
		char *error_response = _create_error_json("400", "Invalid JSON"); 
		char response[BUFSIZ]; 
		snprintf(response, sizeof(response), 
				"HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\n\r\n%s", error_response); 
		send_net(conn, response, strlen(response)); 
		free(error_response); 
		return; 
	}

	cJSON *json_login = cJSON_GetObjectItem(json, "login"); 
	cJSON *json_email = cJSON_GetObjectItem(json, "email");
       	cJSON *json_password = cJSON_GetObjectItem(json, "password");

	if (!cJSON_IsString(json_login) || !cJSON_IsString(json_email) || !cJSON_IsString(json_password)) { 
		printf("Invalid JSON structure. Missing login, email, or password.\n"); 
		char *error_response = _create_error_json("400", "Invalid JSON structure"); 
		char response[BUFSIZ]; 
		snprintf(response, sizeof(response), 
				"HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\n\r\n%s", 
				error_response); 
		send_net(conn, response, strlen(response)); 
		cJSON_Delete(json); 
		free(error_response); 
		return; 
	}

	const char *login = json_login->valuestring; 
	const char *email = json_email->valuestring; 
	const char *password = json_password->valuestring;

	char hashed_password[96]; 
	hash_password(password, hashed_password); 

	const char *param_values[3] = {login, email, hashed_password}; 
	PGresult *res = PQexecParams(pg_conn, 
			"INSERT INTO users (username, email, password_hash) VALUES ($1, $2, $3) RETURNING id", 
			3,  NULL, param_values, NULL, NULL, 0); 
	if (PQresultStatus(res) != PGRES_TUPLES_OK) { 
		fprintf(stderr, "Insert user failed: %s\n", PQerrorMessage(pg_conn)); 
		PQclear(res); 
		char *error_response = _create_error_json("500", "Failed to register user"); 
		char response[BUFSIZ]; 
		snprintf(response, sizeof(response), 
				"HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\n\r\n%s", 
				error_response); 
		send_net(conn, response, strlen(response)); 
		free(error_response); 
		cJSON_Delete(json); 
		return; 
	}

       	char token[33]; 
	generate_token(token); 
	redisContext *redis_conn = redisConnect("127.0.0.1", 6379); 
	if (redis_conn == NULL || redis_conn->err) { 
		if (redis_conn) { 
			fprintf(stderr, "Redis connection error: %s\n", redis_conn->errstr); 
			redisFree(redis_conn); 
		} 
		else { 
			fprintf(stderr, "Redis connection error: can't allocate redis context\n"); 
		} 
		PQclear(res); 
		char *error_response = _create_error_json("500", "Failed to generate token"); 
		char response[BUFSIZ]; 
		snprintf(response, sizeof(response), 
				"HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\n\r\n%s", 
				error_response); 
		send_net(conn, response, strlen(response)); 
		free(error_response); 
		return; 
	} 
	redisCommand(redis_conn, "SET %s %s", token, PQgetvalue(res, 0, 0)); 
	redisFree(redis_conn); 
	cJSON *response_json = cJSON_CreateObject(); 
	cJSON_AddStringToObject(response_json, "token", token); 
	char *json_response = cJSON_Print(response_json); 
	cJSON_Delete(response_json); 
	char response[BUFSIZ]; 
	snprintf(response, sizeof(response), 
			"HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", json_response); 
	send_net(conn, response, strlen(response)); 
	free(json_response); 
	PQclear(res); 
}

cJSON* _create_user_json(char *status, char *login, char *email, char *date_of_register){

	cJSON *user = cJSON_CreateObject();
	cJSON_AddStringToObject(user, "status", status);
	cJSON_AddStringToObject(user, "login", login);
	cJSON_AddStringToObject(user, "email", email);
	cJSON_AddStringToObject(user, "date_of_register", date_of_register);
	return user;
}

char* _create_users_json_response(PGresult *res) { 
	cJSON *users_array = cJSON_CreateArray(); 
	for (int i = 0; i < PQntuples(res); i++) { 
		cJSON *user = _create_user_json("200", PQgetvalue(res, i, 1), 
				PQgetvalue(res, i, 2), PQgetvalue(res, i, 4)); 
		cJSON_AddItemToArray(users_array, user); 
	} 
	char *json_response = cJSON_Print(users_array); 
	cJSON_Delete(users_array); 
	return json_response; 
}

char* _create_error_json(const char *status, const char *message) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", status);
    cJSON_AddStringToObject(json, "message", message);
    char *json_response = cJSON_Print(json);
    cJSON_Delete(json);
    return json_response;
}

