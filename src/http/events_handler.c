#include <stdlib.h>
#include "../../inc/http/events_handler.h"
#include "../../inc/http/response.h"

struct events_handler {
    http_response_t success_response_template,
            forbidden_response_template,
            not_found_response_template,
            not_allowed_response_template,
            fail_response_template;
};

int http_events_handler_create(http_events_handler_t *handler) {
    // TODO
}

int handle_http_event(http_events_handler_t handler, int socket_fd) {
    // TODO
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
