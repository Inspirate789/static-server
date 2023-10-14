#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/header.h"
#include "../../lib/log/log.h"

struct http_header {
    char *name;
    char *value;
};

int http_header_create(http_header_t *header, const char *name, const char *value) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (name == NULL) {
        log_error("name pointer is NULL");
        return EXIT_FAILURE;
    }
    if (value == NULL) {
        log_error("value pointer is NULL");
        return EXIT_FAILURE;
    }

    http_header_t tmp = malloc(sizeof(struct http_header));
    if (tmp == NULL) {
        log_error("http_header_create malloc() struct http_header: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    tmp->name = strdup(name);
    if (tmp->name == NULL) {
        log_error("http_header_create strdup() name: %s", strerror(errno));
        free(tmp);
        return EXIT_FAILURE;
    }

    tmp->value = strdup(value);
    if (tmp->value == NULL) {
        log_error("http_header_create strdup() value: %s", strerror(errno));
        free(tmp->name);
        free(tmp);
        return EXIT_FAILURE;
    }

    *header = tmp;

    return EXIT_SUCCESS;
}

int http_header_create_from_raw(http_header_t *header, const char *raw_header) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (raw_header == NULL) {
        log_error("raw_header pointer is NULL");
        return EXIT_FAILURE;
    }

    char *raw = strdup(raw_header);
    if (raw == NULL) {
        log_error("http_header_create_from_raw strdup() raw_header: %s", strerror(errno));
        return EXIT_FAILURE;
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
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (name == NULL) {
        log_error("name pointer is NULL");
        return EXIT_FAILURE;
    }

    char *tmp = strdup(name);
    if (header->name == NULL) {
        log_error("http_header_set_name strdup(): %s", strerror(errno));
        return EXIT_FAILURE;
    }
    free(header->name);
    header->name = tmp;

    return EXIT_SUCCESS;
}

int http_header_get_name(http_header_t header, char **name) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (name == NULL) {
        log_error("name pointer is NULL");
        return EXIT_FAILURE;
    }

    *name = header->name;

    return EXIT_SUCCESS;
}

int http_header_set_value(http_header_t header, const char *value) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (value == NULL) {
        log_error("value pointer is NULL");
        return EXIT_FAILURE;
    }

    char *tmp = strdup(value);
    if (header->name == NULL) {
        log_error("http_header_set_value strdup(): %s", strerror(errno));
        return EXIT_FAILURE;
    }
    free(header->value);
    header->value = tmp;

    return EXIT_SUCCESS;
}

int http_header_get_value(http_header_t header, char **value) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (value == NULL) {
        log_error("value pointer is NULL");
        return EXIT_FAILURE;
    }

    *value = header->value;

    return EXIT_SUCCESS;
}

int http_header_make_raw(http_header_t header, char **raw_header) {
    if (header == NULL) {
        log_error("header pointer is NULL");
        return EXIT_FAILURE;
    }
    if (raw_header == NULL) {
        log_error("raw_header pointer is NULL");
        return EXIT_FAILURE;
    }

    char *raw = malloc(strlen(header->name) + strlen(header->value) + 2);
    if (raw == NULL) {
        log_error("http_header_make_raw malloc(): %s", strerror(errno));
        return EXIT_FAILURE;
    }

    strncpy(raw, header->name, strlen(header->name));
    strncpy(raw + strlen(header->name), ": ", 2);
    strncpy(raw + strlen(header->name) + 2, header->value, strlen(header->value));

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
