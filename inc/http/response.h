#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "header.h"

typedef const char *http_status_code_t;

http_status_code_t OK                    = "200 OK";
http_status_code_t BAD_REQUEST           = "400 Bad Request";
http_status_code_t FORBIDDEN             = "403 Forbidden";
http_status_code_t NOT_FOUND             = "404 Not Found";
http_status_code_t METHOD_NOT_ALLOWED    = "405 Method Not Allowed";
http_status_code_t INTERNAL_SERVER_ERROR = "500 Internal Server Error";

typedef struct http_response *http_response_t;

int http_response_create(http_response_t *response);
int http_response_set_proto(http_response_t response, const char *proto);
int http_response_set_status_code(http_response_t response, http_status_code_t status_code);
int http_response_set_header(http_response_t response, const char *name, const char *value);
int http_response_set_body(http_response_t response, const char *body);
int http_response_get_raw(http_response_t response, char **raw_response);
void http_response_destroy(http_response_t *response);

#endif //HTTP_RESPONSE_H
