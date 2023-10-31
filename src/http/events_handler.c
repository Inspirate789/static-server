#include <stdlib.h>
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
        return EXIT_FAILURE;
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
    // TODO
}

static int log_http_request(http_request_t request) {
    // TODO
}

static int log_http_response(http_request_t request, http_status_code_t status_code) {
    // TODO
}

int handle_http_event(http_events_handler_t handler, int socket_fd) {
    char *raw_request = NULL;
    int rc = read_http_request(socket_fd, &raw_request);
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
    rc = make_decision(handler->decisions_maker, request, &response);
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

    rc = log_http_response(request, HTTP_OK);
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
