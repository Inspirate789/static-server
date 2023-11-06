#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "events_handler.h"
#include "decisions_maker.h"
#include "request.h"
#include "log.h"

static int read_http_request(int socket_fd, char *raw_request) {
    ssize_t n = read(socket_fd, raw_request, REQUEST_BUFFER_SIZE - 1);
    if (n < 0) {
        log_error("read() from fd %d: %s", socket_fd, strerror(errno));
        return errno;
    }
    raw_request[n] = '\0';

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

    log_debug("REQ %s %s by %s", http_method_mapping(method), path, proto);

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

    clock_t processing_time;
    if ((rc = http_request_compute_processing_time_ms(request, &processing_time)) != EXIT_SUCCESS) {
        return rc;
    }

    if (strcmp(status_code, HTTP_OK) == 0) {
        log_info("\033[0;32m%c%c%c %s %s by %s --- %d ms\033[0m",
            status_code[0], status_code[1], status_code[2], http_method_mapping(method), path, proto, processing_time
        );
    } else {
        log_info("\033[0;31m%c%c%c %s %s by %s --- %d ms\033[0m",
                 status_code[0], status_code[1], status_code[2], http_method_mapping(method), path, proto, processing_time
        );
    }

    return EXIT_SUCCESS;
}

int handle_http_event(int socket_fd) {
    // log_info("process request from client (fd = %d) ...", socket_fd);
    char raw_request[REQUEST_BUFFER_SIZE] = {'\0'};
    int rc = read_http_request(socket_fd, raw_request);
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
    rc = make_decision(request, &response, &status_code);
    if (rc != EXIT_SUCCESS) {
        http_request_destroy(&request);
        return rc;
    }

    rc = http_response_write(response, socket_fd);
    http_response_close_attachment(response);
    http_response_destroy(&response);
    if (rc != EXIT_SUCCESS) {
        http_request_destroy(&request);
        return rc;
    }

    rc = log_http_response(request, status_code);
    http_request_destroy(&request);

    return rc;
}
