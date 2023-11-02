#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/events_handler.h"
#include "../../inc/http/decisions_maker.h"
#include "../../lib/log/log.h"

struct http_events_handler {
    decisions_maker_t decisions_maker;
};

int http_events_handler_create(http_events_handler_t *handler) {
    if (handler == NULL) {
        log_error("handler pointer is NULL");
        return EXIT_FAILURE;
    }

    http_events_handler_t tmp_handler = malloc(sizeof(struct http_events_handler));
    if (tmp_handler == NULL) {
        log_error("http_events_handler_create malloc() struct http_events_handler: %s", strerror(errno));
        return errno;
    }
    
    decisions_maker_t tmp_decisions_maker = NULL;
    int rc = decisions_maker_create(&tmp_decisions_maker);
    if (rc != EXIT_SUCCESS) {
        free(tmp_handler);
        return rc;
    }
    
    tmp_handler->decisions_maker = tmp_decisions_maker;
    *handler = tmp_handler;

    return EXIT_SUCCESS;
}

static int read_http_request(int socket_fd, char **raw_request) {
    ssize_t n = read(socket_fd, *raw_request, REQUEST_BUFFER_SIZE - 1);
    if (n < 0) {
        log_error("read() from fd %d: %s", socket_fd, strerror(errno));
        return errno;
    }
    *raw_request[n] = '\0';

    return EXIT_SUCCESS;
}

static int log_http_request(http_request_t request) {
    char *path = NULL;
    int rc = http_request_get_path(request, &path);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    http_method_t method;
    if ((rc = http_request_get_method(request, &method)) != EXIT_SUCCESS) {
        return rc;
    }

    http_proto_t proto = NULL;
    if ((rc = http_request_get_proto(request, &proto)) != EXIT_SUCCESS) {
        return rc;
    }

    log_info("REQ %s %s by %s", method, path, proto);

    return EXIT_SUCCESS;
}

static int log_http_response(http_request_t request, http_status_code_t status_code) {
    char *path = NULL;
    int rc = http_request_get_path(request, &path);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    http_method_t method;
    if ((rc = http_request_get_method(request, &method)) != EXIT_SUCCESS) {
        return rc;
    }

    http_proto_t proto = NULL;
    if ((rc = http_request_get_proto(request, &proto)) != EXIT_SUCCESS) {
        return rc;
    }

    if (status_code == HTTP_OK) {
        log_info("\033[0;32m%c%c%c %s %s by %s\033[0m", status_code[0], status_code[1], status_code[2], method, path, proto);
    } else {
        log_info("\033[0;31m%c%c%c %s %s by %s\033[0m", status_code[0], status_code[1], status_code[2], method, path, proto);
    }

    return EXIT_SUCCESS;
}

int handle_http_event(http_events_handler_t handler, int socket_fd) {
    // log_info("process request from client (fd = %d) ...", socket_fd);
    char raw_request[REQUEST_BUFFER_SIZE] = {'\0'};
    int rc = read_http_request(socket_fd, (char **)&raw_request);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    http_request_t request = NULL;
    rc = http_request_create(&request, raw_request);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    rc = log_http_request(request);
    if (rc != EXIT_SUCCESS) {
        http_request_destroy(&request);
        return rc;
    }

    http_response_t response = NULL;
    http_status_code_t status_code = NULL;
    rc = make_decision(handler->decisions_maker, request, &response, &status_code);
    if (rc != EXIT_SUCCESS) {
        http_request_destroy(&request);
        return rc;
    }

    rc = http_response_write(response, socket_fd);
    http_response_destroy(&response);
    if (rc != EXIT_SUCCESS) {
        http_request_destroy(&request);
        return rc;
    }

    rc = log_http_response(request, status_code);
    http_request_destroy(&request);

    return rc;
}

void http_events_handler_destroy(http_events_handler_t *handler) {
    if (handler == NULL || *handler == NULL) {
        return;
    }
    decisions_maker_destroy(&(*handler)->decisions_maker);
    free(*handler);
    *handler = NULL;
}
