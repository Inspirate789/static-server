#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "header.h"
#include "request.h"

#define HTTP_OK                    "200 OK"
#define HTTP_FORBIDDEN             "403 Forbidden"
#define HTTP_NOT_FOUND             "404 Not Found"
#define HTTP_METHOD_NOT_ALLOWED    "405 Method Not Allowed"
#define HTTP_INTERNAL_SERVER_ERROR "500 Internal Server Error"

#define HTTP_1_0 "HTTP/1.0"
#define HTTP_1_1 "HTTP/1.1"

typedef char *http_status_code_t;

typedef struct http_response *http_response_t;

int http_response_create(http_response_t *response);
int http_response_set_proto(http_response_t response, http_proto_t proto);
int http_response_set_status_code(http_response_t response, http_status_code_t status_code);
int http_response_set_header(http_response_t response, const char *name, const char *value);
int http_response_set_body(http_response_t response, const char *body);
int http_response_set_attachment(http_response_t response, int fd);
int http_response_write(http_response_t response, int fd);
void http_response_destroy(http_response_t *response);

#endif //HTTP_RESPONSE_H
