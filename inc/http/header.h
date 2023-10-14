#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

typedef struct http_header *http_header_t;

int http_header_create(http_header_t *header, const char *name, const char *value);
int http_header_create_from_raw(http_header_t *header, const char *raw_header);
int http_header_set_name(http_header_t header, const char *name);
int http_header_get_name(http_header_t header, char **name);
int http_header_set_value(http_header_t header, const char *value);
int http_header_get_value(http_header_t header, char **value);
int http_header_make_raw(http_header_t header, char **raw_header);
void http_header_destroy_raw(char **raw_header);
void http_header_destroy(http_header_t *header);

#endif //HTTP_HEADER_H
