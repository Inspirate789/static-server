#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/headers.h"
#include "../../lib/log/log.h"

#define MEMORY_EXPANSION_RATE 1.25

struct http_headers {
    http_header_t *headers;
    size_t size;
    size_t capacity;
};

int http_headers_create(http_headers_t *headers, const size_t capacity) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }

    http_header_t *tmp_headers = malloc(capacity * sizeof(http_header_t));
    if (tmp_headers == NULL) {
        log_error("http_headers_create malloc() headers: %s", strerror(errno));
        return errno;
    }

    http_headers_t tmp = malloc(sizeof(struct http_headers));
    if (tmp == NULL) {
        log_error("http_headers_create malloc() struct http_headers: %s", strerror(errno));
        free(tmp_headers);
        return EXIT_FAILURE;
    }

    tmp->headers = tmp_headers;
    tmp->size = 0;
    tmp->capacity = capacity;
    *headers = tmp;

    return EXIT_SUCCESS;
}

static int http_headers_set_capacity(http_headers_t headers, const size_t capacity) {
    http_header_t *tmp = realloc(headers->headers, capacity * sizeof(http_header_t));
    if (tmp == NULL) {
        log_error("http_headers_set_capacity realloc() headers: %s", strerror(errno));
        return errno;
    }
    headers->headers = tmp;
    headers->capacity = capacity;

    return EXIT_SUCCESS;
}

static int http_headers_expand(http_headers_t headers) {
    return http_headers_set_capacity(headers, (size_t)((double)headers->capacity * MEMORY_EXPANSION_RATE));
}

int http_headers_append(http_headers_t headers, const http_header_t header) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }

    if (headers->size == headers->capacity) {
        int rc = http_headers_expand(headers);
        if (rc != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    headers->headers[headers->size++] = header;

    return EXIT_SUCCESS;
}

int http_headers_set_header(http_headers_t headers, const char *name, const char *value) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }

    http_header_t header = NULL;
    int rc = http_header_create(&header, name, value);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    return http_headers_append(headers, header);
}

int http_headers_create_header(http_headers_t headers, const char *raw_header) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }

    http_header_t header = NULL;
    int rc = http_header_create_from_raw(&header, raw_header);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    return http_headers_append(headers, header);
}

int http_headers_compress(http_headers_t headers) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }

    return http_headers_set_capacity(headers, headers->size);
}

int http_headers_find_header(http_headers_t headers, const char *name, char **value) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }
//    if (name == NULL) {
//        log_error("name pointer is NULL");
//        return EXIT_FAILURE;
//    }
//    if (value == NULL) {
//        log_error("value pointer is NULL");
//        return EXIT_FAILURE;
//    }

    char *found = NULL;
    for (size_t i = 0; i < headers->size; i++) {
        char *cur_name = NULL;
        int rc = http_header_get_name(headers->headers[i], &cur_name);
        if (rc != EXIT_SUCCESS) {
            return rc;
        }
        if (strcmp(name, cur_name) == 0) {
            char *cur_value = NULL;
            rc = http_header_get_value(headers->headers[i], &cur_value);
            if (rc != EXIT_SUCCESS) {
                return rc;
            }
            found = cur_value;
            break;
        }
    }
    if (found == NULL) {
        return HTTP_HEADER_NOT_FOUND;
    }
    *value = found;

    return EXIT_SUCCESS;
}

int http_headers_truncate(http_headers_t headers, const size_t new_size) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }
    if (new_size < headers->size) {
        for (size_t i = new_size; i < headers->size; i++) {
            http_header_destroy(&headers->headers[i]);
        }
        headers->size = new_size;
    }

    return EXIT_SUCCESS;
}

int http_headers_size(http_headers_t headers, size_t *size) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }
//    if (size == NULL) {
//        log_error("size pointer is NULL");
//        return EXIT_FAILURE;
//    }
    *size = headers->size;

    return EXIT_SUCCESS;
}

int http_headers_at(http_headers_t headers, const size_t index, http_header_t *header) {
//    if (headers == NULL) {
//        log_error("headers pointer is NULL");
//        return EXIT_FAILURE;
//    }
//    if (header == NULL) {
//        log_error("header pointer is NULL");
//        return EXIT_FAILURE;
//    }
//    if (index >= headers->size) {
//        log_error("headers index out of range");
//        return EXIT_FAILURE;
//    }
    *header = headers->headers[index];

    return EXIT_SUCCESS;
}

void http_headers_destroy(http_headers_t *headers) {
    if (headers == NULL || *headers == NULL) {
        return;
    }
    for (size_t i = 0; i < (*headers)->size; i++) {
        http_header_destroy(&(*headers)->headers[i]);
    }
    free((*headers)->headers);
    free(*headers);
    *headers = NULL;
}
