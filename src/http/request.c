#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include "../../inc/http/request.h"
#include "../../inc/http/headers.h"
#include "../../lib/log/log.h"

struct http_request {
    http_method_t method;
    char *path;
    http_proto_t proto;
    http_headers_t headers;
    char *body;
    clock_t start_time;
};

static size_t substrings_count(const char *str, const char *substr){
    size_t count = 0;
    while((str = strstr(str, substr)) != NULL) {
        count++;
        str++;
    }

    return count;
}

static int http_request_parse_lines(const char *raw_request, char ***lines, size_t *n) {
    char *raw = strdup(raw_request);
    if (raw == NULL) {
        log_error("http_request_parse_lines strdup() raw_request: %s", strerror(errno));
        return errno;
    }

    size_t max_lines_count = substrings_count(raw, "\r\n") + 1;
    char **tokens = malloc(max_lines_count * sizeof(char*));
    if (tokens == NULL) {
        log_error("http_request_parse_lines malloc() lines: %s", strerror(errno));
        return errno;
    }
    char *token = strtok(raw, "\r\n");
    size_t tokens_count = 0;

    while (token != NULL) {
        tokens[tokens_count++] = strdup(token);
        token = strtok(NULL, "\r\n");
    }

    free(raw);
    *lines = tokens;
    *n = tokens_count;

    return EXIT_SUCCESS;
}

static void http_request_free_lines(char **lines, size_t n) {
    for (size_t i = 0; i < n; i++) {
        free(lines[i]);
    }
    free(lines);
}

static int http_request_parse_method(const char *method) {
    const char *http_methods[] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "PATCH",
    };

    for (int i = GET; i < PATCH; i++) {
        if (strcmp(method, http_methods[i]) == 0) {
            return i;
        }
    }

    log_error("http request contains unknown method");
    return INVALID_HTTP_REQUEST;
}

static int http_request_parse_first_line(http_request_t request, char *line) {
    char *start = line, *end = strstr(line, " ");
    if (end == NULL) {
        log_error("request first line is invalid");
        return INVALID_HTTP_REQUEST;
    }
    *end = '\0';
    request->method = http_request_parse_method(start);
    *end = ' ';
    if (request->method == INVALID_HTTP_REQUEST) {
        return INVALID_HTTP_REQUEST;
    }

    start = end + 1;
    end = strstr(start, " ");
    if (end == NULL) {
        log_error("request first line is invalid");
        return INVALID_HTTP_REQUEST;
    }
    *end = '\0';
    request->path = strdup(start);
    *end = ' ';
    if (request->path == NULL) {
        log_error("http_request_parse_first_line strdup() path: %s", strerror(errno));
        return errno;
    }

    start = end + 1;
    request->proto = strdup(start);
    if (request->path == NULL) {
        log_error("http_request_parse_first_line strdup() path: %s", strerror(errno));
        free(request->path);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int http_request_parse_headers_and_body(http_request_t request, char **lines, const size_t n) {
    int rc = EXIT_SUCCESS;
    for (size_t i = 1; i < n - 1; i++) {
        if ((rc = http_headers_create_header(request->headers, lines[i])) != EXIT_SUCCESS) {
            return rc;
        }
    }

    bool body_exists = false;
    char *value = NULL;
    rc = http_headers_find_header(request->headers, "Content-Type", &value);
    if (rc == EXIT_SUCCESS) {
        rc = http_headers_find_header(request->headers, "Content-Length", &value);
        if (rc == EXIT_SUCCESS) {
            body_exists = true;
        }
    } else if (rc != HTTP_HEADER_NOT_FOUND) {
        return rc;
    }

    if (body_exists) {
        request->body = strdup(lines[n - 1]);
        if (request->body == NULL) {
            log_error("http_request_create strdup() body: %s", strerror(errno));
            return errno;
        }
    } else {
        request->body = NULL;
        if ((rc = http_headers_create_header(request->headers, lines[n - 1])) != EXIT_SUCCESS) {
            return rc;
        }
    }

    return EXIT_SUCCESS;
}

int http_request_create(http_request_t *request, const char *raw_request) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (raw_request == NULL) {
        log_error("raw_request pointer is NULL");
        return EXIT_FAILURE;
    }

    clock_t start_time = clock();

    char **lines = NULL;
    size_t n = 0;
    int rc = http_request_parse_lines(raw_request, &lines, &n);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }
    if (n == 0) {
        log_error("raw_request is invalid");
        rc = INVALID_HTTP_REQUEST;
        goto free_lines;
    }

    http_request_t tmp_request = malloc(sizeof(struct http_request));
    if (tmp_request == NULL) {
        log_error("http_request_create malloc() http_request: %s", strerror(errno));
        rc = EXIT_FAILURE;
        goto free_lines;
    }
    tmp_request->start_time = start_time;

    if ((rc = http_request_parse_first_line(tmp_request, lines[0])) != EXIT_SUCCESS) {
        goto free_request;
    }

    if ((rc = http_headers_create(&tmp_request->headers, AVERAGE_HTTP_HEADERS_COUNT)) != EXIT_SUCCESS) {
        goto free_request_first_line;
    }

    if ((rc = http_request_parse_headers_and_body(tmp_request, lines, n)) == EXIT_SUCCESS) {
        *request = tmp_request;
        goto exit;
    }

    http_headers_destroy(&tmp_request->headers);
free_request_first_line:
    free(tmp_request->path);
    free(tmp_request->proto);
free_request:
    free(tmp_request);
free_lines:
    http_request_free_lines(lines, n);
exit:
    return rc;
}

int http_request_compute_processing_time_ms(http_request_t request, clock_t *diff) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (diff == NULL) {
        log_error("diff pointer is NULL");
        return EXIT_FAILURE;
    }

    *diff = clock() - request->start_time;

    return EXIT_SUCCESS;
}

int http_request_get_method(http_request_t request, http_method_t *method) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (method == NULL) {
        log_error("path pointer is NULL");
        return EXIT_FAILURE;
    }

    *method = request->method;

    return EXIT_SUCCESS;
}

int http_request_get_path(http_request_t request, char **path) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (path == NULL) {
        log_error("path pointer is NULL");
        return EXIT_FAILURE;
    }

    *path = request->path;

    return EXIT_SUCCESS;
}

int http_request_get_proto(http_request_t request, char **proto) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (proto == NULL) {
        log_error("proto pointer is NULL");
        return EXIT_FAILURE;
    }

    *proto = request->proto;

    return EXIT_SUCCESS;
}

int http_request_find_header(http_request_t request, const char *name, char **value) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }

    return http_headers_find_header(request->headers, name, value);
}

int http_request_get_body(http_request_t request, char **body) {
    if (request == NULL) {
        log_error("request pointer is NULL");
        return EXIT_FAILURE;
    }
    if (body == NULL) {
        log_error("body pointer is NULL");
        return EXIT_FAILURE;
    }

    *body = request->body;

    return EXIT_SUCCESS;
}

void http_request_destroy(http_request_t *request) {
    if (request == NULL || *request == NULL) {
        return;
    }
    free((*request)->path);
    free((*request)->proto);
    http_headers_destroy(&(*request)->headers);
    free((*request)->body);
    free(*request);
    *request = NULL;
}
