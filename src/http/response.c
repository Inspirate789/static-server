#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/response.h"
#include "../../inc/http/headers.h"
#include "../../lib/fs/fs.h"
#include "../../lib/log/log.h"

struct http_response {
    char *proto;
    http_status_code_t status_code;
    http_headers_t headers;
    char *body;
    int attachment_fd;
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

    tmp_response->attachment_fd = -1;
    *response = tmp_response;

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

int http_response_set_attachment(http_response_t response, int fd) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }

    free(response->body);
    response->attachment_fd = fd;

    return EXIT_SUCCESS;
}

static int http_response_check(http_response_t response) {
    if (response == NULL) {
        log_error("response pointer is NULL");
        return EXIT_FAILURE;
    }
    if (response->proto == NULL) {
        log_error("response proto is not set");
        return EXIT_FAILURE;
    }
    if (response->status_code == NULL) {
        log_warn("response status code is not set; change to %s", OK);
        response->status_code = OK;
    }

    return EXIT_SUCCESS;
}

static int http_response_write_status_line(http_response_t response, int fd) {
    if (write(fd, response->proto, strlen(response->proto)) != strlen(response->proto)) {
        log_error("http_response_write write proto %s to fd %d: %s", response->proto, fd, strerror(errno));
        return EXIT_FAILURE;
    }

    if (write(fd, " ", 1) != 1) {
        log_error("http_response_write write space to fd %d: %s", fd, strerror(errno));
        return EXIT_FAILURE;
    }

    if (write(fd, response->status_code, strlen(response->status_code)) != strlen(response->status_code)) {
        log_error("http_response_write write status_code %s to fd %d: %s", response->status_code, fd, strerror(errno));
        return EXIT_FAILURE;
    }

    if (write(fd, "\r\n", 2) != 2) {
        log_error("http_response_write write \\r\\n to fd %d: %s", fd, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int http_response_write_headers(http_headers_t headers, int fd) {
    int rc;
    size_t headers_count = 0;
    if ((rc = http_headers_size(headers, &headers_count)) != EXIT_SUCCESS) {
        return rc;
    }

    for (size_t i = 0; i < headers_count; i++) {
        http_header_t cur_header;
        if ((rc = http_headers_at(headers, i, &cur_header)) != EXIT_SUCCESS) {
            return rc;
        }

        char *raw_header = NULL;
        if ((rc = http_header_make_raw(cur_header, &raw_header)) != EXIT_SUCCESS) {
            return rc;
        }

        ssize_t n = write(fd, raw_header, strlen(raw_header));
        http_header_destroy_raw(&raw_header);
        if (n != strlen(raw_header)) {
            log_error("http_response_write write header %s to fd %d: %s", raw_header, fd, strerror(errno));
            return EXIT_FAILURE;
        }


        if (write(fd, "\r\n", 2) != 2) {
            log_error("http_response_write write \\r\\n to fd %d: %s", fd, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static int http_response_write_body(http_response_t response, int fd) {
    if (response->body != NULL || response->attachment_fd != -1) {
        if (write(fd, "\r\n\r\n", 4) != 4) {
            log_error("http_response_write write \\r\\n\\r\\n to fd %d: %s", fd, strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
        return EXIT_SUCCESS;
    }

    if (response->body != NULL) {
        if (write(fd, response->body, strlen(response->body)) != strlen(response->body)) {
            log_error("http_response_write write body to fd %d: %s", fd, strerror(errno));
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    int rc;
    if (response->attachment_fd != -1) {
        if ((rc = copy_file(response->attachment_fd, fd)) != EXIT_SUCCESS) {
            return rc;
        }
    }

    return EXIT_SUCCESS;
}

int http_response_write(http_response_t response, int fd) {
    int rc;
    if ((rc = http_response_check(response)) != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_write_status_line(response, fd)) != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_write_headers(response->headers, fd)) != EXIT_SUCCESS) {
        return rc;
    }

    return http_response_write_body(response, fd);
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
