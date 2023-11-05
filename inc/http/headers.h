#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include <stdlib.h>
#include "header.h"

#define AVERAGE_HTTP_HEADERS_COUNT 16

#define HTTP_HEADER_NOT_FOUND (-1)

typedef struct http_headers *http_headers_t;

int http_headers_create(http_headers_t *headers, const size_t capacity);
int http_headers_append(http_headers_t headers, const http_header_t header);
int http_headers_set_header(http_headers_t headers, const char *name, const char *value); // TODO: search for replace
int http_headers_create_header(http_headers_t headers, const char *raw_header);
int http_headers_compress(http_headers_t headers);
int http_headers_find_header(http_headers_t headers, const char *name, char **value);
int http_headers_truncate(http_headers_t headers, const size_t new_size);
int http_headers_size(http_headers_t headers, size_t *size);
int http_headers_at(http_headers_t headers, const size_t index, http_header_t *header);
void http_headers_destroy(http_headers_t *headers);

#endif //HTTP_HEADERS_H
