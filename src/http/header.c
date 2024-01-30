#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "header.h"
#include "log.h"

struct http_header {
    char *name;
    char *value;
};

int http_header_create(http_header_t *header, const char *name, const char *value) {
    http_header_t tmp_header = malloc(sizeof(struct http_header));
    if (tmp_header == NULL) {
        log_error("http_header_create malloc() struct http_header: %s", strerror(errno));
        return errno;
    }

    tmp_header->name = strdup(name);
    if (tmp_header->name == NULL) {
        log_error("http_header_create strdup() name: %s", strerror(errno));
        free(tmp_header);
        return EXIT_FAILURE;
    }

    tmp_header->value = strdup(value);
    if (tmp_header->value == NULL) {
        log_error("http_header_create strdup() value: %s", strerror(errno));
        free(tmp_header->name);
        free(tmp_header);
        return EXIT_FAILURE;
    }

    *header = tmp_header;

    return EXIT_SUCCESS;
}

int http_header_create_from_raw(http_header_t *header, const char *raw_header) {
    char *raw = strdup(raw_header);
    if (raw == NULL) {
        log_error("http_header_create_from_raw strdup() raw_header: %s", strerror(errno));
        return errno;
    }

    char *value = strstr(raw, ": ") + 2;
    if (value == NULL) {
        log_error("get invalid http header: '%s'", raw);
        free(raw);
        return EXIT_FAILURE;
    }
    if (value >= raw + strlen(raw)) {
        log_error("http header hasn't any value: '%s'", raw);
        free(raw);
        return EXIT_FAILURE;
    }
    *(value - 2) = '\0';

    int rc = http_header_create(header, raw, value);
    free(raw);

    return rc;
}

int http_header_set_name(http_header_t header, const char *name) {
    char *tmp = strdup(name);
    if (header->name == NULL) {
        log_error("http_header_set_name strdup(): %s", strerror(errno));
        return errno;
    }
    free(header->name);
    header->name = tmp;

    return EXIT_SUCCESS;
}

int http_header_get_name(http_header_t header, char **name) {
    *name = header->name;
    return EXIT_SUCCESS;
}

int http_header_set_value(http_header_t header, const char *value) {
    char *tmp = strdup(value);
    if (header->name == NULL) {
        log_error("http_header_set_value strdup(): %s", strerror(errno));
        return errno;
    }
    free(header->value);
    header->value = tmp;

    return EXIT_SUCCESS;
}

int http_header_get_value(http_header_t header, char **value) {
    *value = header->value;
    return EXIT_SUCCESS;
}

int http_header_make_raw(http_header_t header, char **raw_header) {
    char *raw = malloc(strlen(header->name) + strlen(header->value) + 3);
    if (raw == NULL) {
        log_error("http_header_make_raw malloc(): %s", strerror(errno));
        return errno;
    }

    strcpy(raw, header->name);
    strcpy(raw + strlen(header->name), ": ");
    strcpy(raw + strlen(header->name) + 2, header->value);
    raw[strlen(header->name) + strlen(header->value) + 2] = '\0';

    *raw_header = raw;

    return EXIT_SUCCESS;
}

void http_header_destroy_raw(char **raw_header) {
    if (raw_header == NULL || *raw_header == NULL) {
        return;
    }
    free(*raw_header);
    *raw_header = NULL;
}

void http_header_destroy(http_header_t *header) {
    if (header == NULL || *header == NULL) {
        return;
    }
    free((*header)->name);
    free((*header)->value);
    free(*header);
    *header = NULL;
}
