#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/events_handler.h"
#include "../../inc/http/response.h"
#include "../../lib/log/log.h"

struct http_events_handler {
    http_response_t success_response_template,
            forbidden_response_template,
            not_found_response_template,
            not_allowed_response_template,
            fail_response_template;
};

static int setup_http_response_template(http_response_t *response) {
    http_response_t tmp_response = NULL;
    int rc = http_response_create(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_proto(tmp_response, HTTP_1_1)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_success_template(http_events_handler_t handler) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_OK)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    handler->success_response_template = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_forbidden_template(http_events_handler_t handler) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_FORBIDDEN)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    handler->forbidden_response_template = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_not_found_template(http_events_handler_t handler) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_NOT_FOUND)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    handler->not_found_response_template = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_not_allowed_template(http_events_handler_t handler) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_METHOD_NOT_ALLOWED)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    handler->not_allowed_response_template = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_fail_template(http_events_handler_t handler) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_INTERNAL_SERVER_ERROR)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    handler->fail_response_template = tmp_response;

    return EXIT_SUCCESS;
}

static int http_events_handler_setup_templates(http_events_handler_t handler) {
    int rc = http_events_handler_setup_success_template(handler);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_events_handler_setup_forbidden_template(handler)) != EXIT_SUCCESS) {
        http_response_destroy(&handler->success_response_template);
        return rc;
    }

    if ((rc = http_events_handler_setup_not_found_template(handler)) != EXIT_SUCCESS) {
        http_response_destroy(&handler->success_response_template);
        http_response_destroy(&handler->forbidden_response_template);
        return rc;
    }

    if ((rc = http_events_handler_setup_not_allowed_template(handler)) != EXIT_SUCCESS) {
        http_response_destroy(&handler->success_response_template);
        http_response_destroy(&handler->forbidden_response_template);
        http_response_destroy(&handler->not_found_response_template);
        return rc;
    }

    if ((rc = http_events_handler_setup_fail_template(handler)) != EXIT_SUCCESS) {
        http_response_destroy(&handler->success_response_template);
        http_response_destroy(&handler->forbidden_response_template);
        http_response_destroy(&handler->not_found_response_template);
        http_response_destroy(&handler->not_allowed_response_template);
        return rc;
    }

    return EXIT_SUCCESS;
}

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

    int rc = http_events_handler_setup_templates(tmp_handler);
    if (rc != EXIT_SUCCESS) {
        free(tmp_handler);
        return rc;
    }

    *handler = tmp_handler;

    return EXIT_SUCCESS;
}

int handle_http_event(http_events_handler_t handler, int socket_fd) {
    // TODO
    // read request
    // make request
    // solution
    // make response
    // send response
}

void http_events_handler_destroy(http_events_handler_t *handler) {
    if (handler == NULL || *handler == NULL) {
        return;
    }
    http_response_destroy(&(*handler)->success_response_template);
    http_response_destroy(&(*handler)->forbidden_response_template);
    http_response_destroy(&(*handler)->not_found_response_template);
    http_response_destroy(&(*handler)->not_allowed_response_template);
    http_response_destroy(&(*handler)->fail_response_template);
    free(*handler);
    *handler = NULL;
}
