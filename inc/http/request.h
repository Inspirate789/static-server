#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>

#define INVALID_HTTP_REQUEST (-2)

typedef enum http_method {
    GET,        // The GET method requests a representation of the specified resource. Requests using GET should only retrieve data.
    HEAD,       // The HEAD method asks for a response identical to a GET request, but without the response body.
    POST,       // The POST method submits an entity to the specified resource, often causing a change in state or side effects on the server.
    PUT,        // The PUT method replaces all current representations of the target resource with the request payload.
    DELETE,     // The DELETE method deletes the specified resource.
    CONNECT,    // The CONNECT method establishes a tunnel to the server identified by the target resource.
    OPTIONS,    // The OPTIONS method describes the communication options for the target resource.
    TRACE,      // The TRACE method performs a message loop-back test along the path to the target resource.
    PATCH,      // The PATCH method applies partial modifications to a resource.
} http_method_t;

static inline char *http_method_mapping(http_method_t method) {
    char *mapping[9] = {
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
    return mapping[method];
}

typedef char *http_proto_t;

typedef struct http_request *http_request_t;

int http_request_create(http_request_t *request, const char *raw_request);
int http_request_compute_processing_time_ms(http_request_t request, clock_t *diff);
int http_request_get_method(http_request_t request, http_method_t *method);
int http_request_get_path(http_request_t request, char **path);
int http_request_get_proto(http_request_t request, char **proto);
int http_request_find_header(http_request_t request, const char *name, char **value);
int http_request_get_body(http_request_t request, char **body);
// int http_request_get_headers(http_request_t request, http_headers_t *headers);
void http_request_destroy(http_request_t *request);

#endif //HTTP_REQUEST_H
