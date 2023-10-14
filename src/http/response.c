#include <string.h>
#include <errno.h>
#include "../../inc/http/response.h"
#include "../../inc/http/headers.h"
#include "../../lib/log/log.h"

struct http_response {
    char *proto;
    http_status_code_t status_code;
    http_headers_t headers;
    char *body;
};

int http_response_create(http_response_t *response) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }

    http_response_t tmp_response = calloc(1, sizeof(struct http_response));
    if (tmp_response == NULL) {
        log_error("http_response_create malloc() response: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    int rc = http_headers_create(&tmp_response->headers, AVERAGE_HTTP_HEADERS_COUNT);
    if (rc != EXIT_SUCCESS) {
        free(tmp_response);
    }

    return rc;
}

int http_response_set_proto(http_response_t response, const char *proto) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }
    if (proto == NULL) {
        log_error("proto pointer is NULL");
        return EXIT_FAILURE;
    }

    char *tmp_proto = strdup(proto);
    if (tmp_proto == NULL) {
        log_error("http_response_set_body strdup() body: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    free(response->proto);
    response->proto = tmp_proto;

    return EXIT_SUCCESS;
}

int http_response_set_status_code(http_response_t response, http_status_code_t status_code) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }
    if (status_code == NULL) {
        log_error("status_code is NULL");
        return EXIT_FAILURE;
    }

    response->status_code = status_code;

    return EXIT_SUCCESS;
}

int http_response_set_header(http_response_t response, const char *name, const char *value) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }

    return http_headers_set_header(response->headers, name, value);
}

int http_response_set_body(http_response_t response, const char *body) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }
    if (body == NULL) {
        log_error("body pointer is NULL");
        return EXIT_FAILURE;
    }

    char *tmp_body = strdup(body);
    if (tmp_body == NULL) {
        log_error("http_response_set_body strdup() body: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    free(response->body);
    response->body = tmp_body;

    return EXIT_SUCCESS;
}

int http_response_get_raw(http_response_t response, char **raw_response) {
     // TODO
}

void http_response_destroy(http_response_t *response) {
    if (response == NULL || *response == NULL) {
        return;
    }
    free((*response)->proto);
    http_headers_destroy(&(*response)->headers);
    free((*response)->body);
    free(*response);
    *response = NULL;
}
