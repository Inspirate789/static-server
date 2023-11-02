#ifndef HTTP_EVENTS_HANDLER_H
#define HTTP_EVENTS_HANDLER_H

#define REQUEST_BUFFER_SIZE 1024

typedef struct http_events_handler *http_events_handler_t;

int http_events_handler_create(http_events_handler_t *handler);
int handle_http_event(http_events_handler_t handler, int socket_fd);
void http_events_handler_destroy(http_events_handler_t *handler);

#endif //HTTP_EVENTS_HANDLER_H
