#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "decisions_maker.h"
#include "fs.h"
#include "log.h"

typedef int (*setup_response_template_t)(http_response_t*, http_proto_t);

static int setup_http_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = http_response_create(&tmp_response);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_proto(tmp_response, proto)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_success_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_OK)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_forbidden_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_FORBIDDEN)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_not_found_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_NOT_FOUND)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_not_allowed_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_METHOD_NOT_ALLOWED)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_fail_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_INTERNAL_SERVER_ERROR)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int setup_not_implemented_response_template(http_response_t *response, http_proto_t proto) {
    http_response_t tmp_response = NULL;
    int rc = setup_http_response_template(&tmp_response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if ((rc = http_response_set_status_code(tmp_response, HTTP_NOT_IMPLEMENTED)) != EXIT_SUCCESS) {
        http_response_destroy(&tmp_response);
        return rc;
    }

    *response = tmp_response;

    return EXIT_SUCCESS;
}

static int parse_http_request(http_request_t request, char **proto, char **path, http_method_t *method) {
    int rc = http_request_get_proto(request, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }
    if ((rc = http_request_get_method(request, method)) != EXIT_SUCCESS) {
        return rc;
    }

    return http_request_get_path(request, path);
}

static int validate_path(const char *path) {
    if (strstr(path, "/..") != NULL) { // not needed
        return EXIT_FAILURE;
    }

    if (strncmp(path, STATIC_PATH, strlen(STATIC_PATH)) != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int detect_content_type(const char *path, char **content_type) {
    size_t len = strlen(path);
    if (len >= 5 && strcmp(path + len - 5, ".html") == 0) {
        *content_type = "text/html";
        return EXIT_SUCCESS;
    } else if (len >= 4 && strcmp(path + len - 4, ".css") == 0) {
        *content_type = "text/css";
        return EXIT_SUCCESS;
    } else if (len >= 3 && strcmp(path + len - 3, ".js") == 0) {
        *content_type = "text/javascript";
        return EXIT_SUCCESS;
    } else if (len >= 4 && strcmp(path + len - 4, ".png") == 0) {
        *content_type = "image/png";
        return EXIT_SUCCESS;
    } else if (len >= 4 && strcmp(path + len - 4, ".jpg") == 0) {
        *content_type = "image/jpg";
        return EXIT_SUCCESS;
    } else if (len >= 5 && strcmp(path + len - 5, ".jpeg") == 0) {
        *content_type = "image/jpe";
        return EXIT_SUCCESS;
    } else if (len >= 4 && strcmp(path + len - 4, ".gif") == 0) {
        *content_type = "image/gif";
        return EXIT_SUCCESS;
    } else if (len >= 4 && strcmp(path + len - 4, ".svg") == 0) {
        *content_type = "image/svg";
        return EXIT_SUCCESS;
    } else if (len >= 4 && (strcmp(path + len - 4, ".swf") == 0 || strcmp(path + len - 4, ".mp4") == 0)) {
        *content_type = "application/x-shockwave-flash";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

static setup_response_template_t select_response_setup_func(http_status_code_t status_code) {
    return status_code == HTTP_OK ?                     setup_success_response_template     : \
           status_code == HTTP_FORBIDDEN ?              setup_forbidden_response_template   : \
           status_code == HTTP_NOT_FOUND ?              setup_not_found_response_template   : \
           status_code == HTTP_METHOD_NOT_ALLOWED ?     setup_not_allowed_response_template : \
           status_code == HTTP_INTERNAL_SERVER_ERROR ?  setup_fail_response_template        : \
                                                        setup_not_implemented_response_template;
}

typedef struct {
    http_status_code_t *status_code;
    char *path;
    http_proto_t proto;
    bool need_body;
    char *content_type;
    size_t content_length;
    bool already_handled;
} http_response_data_t;

static int make_response(http_response_data_t data, http_response_t *response) {
    if (data.already_handled) {
        http_response_destroy(response);
    }

    setup_response_template_t setup_response_template = select_response_setup_func(*data.status_code);
    int rc = setup_response_template(response, data.proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    if (!data.already_handled && *data.status_code == HTTP_OK) {
        if (http_response_set_header(*response, "Content-Type", data.content_type) != EXIT_SUCCESS) {
            *data.status_code = HTTP_INTERNAL_SERVER_ERROR;
            data.already_handled = true;
            return make_response(data, response);
        }
        char buf[20] = {'\0'};
        snprintf(buf, 20, "%lu", data.content_length);
        if (http_response_set_header(*response, "Content-Length", buf) != EXIT_SUCCESS) {
            *data.status_code = HTTP_INTERNAL_SERVER_ERROR;
            data.already_handled = true;
            return make_response(data, response);
        }

        if (data.need_body) {
            int fd = open(data.path, O_RDONLY);
            if (fd == -1) {
                log_error("make_decision(): %s", strerror(errno));
                *data.status_code = HTTP_INTERNAL_SERVER_ERROR;
                data.already_handled = true;
                return make_response(data, response);
            }
            if (http_response_set_attachment(*response, fd) != EXIT_SUCCESS) {
                *data.status_code = HTTP_INTERNAL_SERVER_ERROR;
                data.already_handled = true;
                return make_response(data, response);
            }
        }
    }

    return EXIT_SUCCESS;
}

int make_decision(http_request_t request, http_response_t *response, http_status_code_t *status_code) {
    http_response_data_t data = {
        .status_code = status_code,
        .path = NULL,
        .proto = HTTP_1_1,
        .need_body = false,
        .content_type = NULL,
        .already_handled = false,
    };
    http_method_t method;
    *status_code = HTTP_OK;

    int rc = parse_http_request(request, &data.proto, &data.path, &method);
    if (rc != EXIT_SUCCESS) {
        *status_code = HTTP_INTERNAL_SERVER_ERROR;
        goto response;
    }

    switch (method) {
        case GET:
            data.need_body = true;
            break;
        case HEAD:
            data.need_body = false;
            break;
        default:
            *status_code = HTTP_METHOD_NOT_ALLOWED;
            goto response;
    }

    if (validate_path(data.path) != EXIT_SUCCESS) {
        *status_code = HTTP_FORBIDDEN;
        goto response;
    }

    if (strlen(data.path) == strlen(STATIC_PATH) ||
        (strlen(data.path) - strlen(STATIC_PATH) == 1 && data.path[strlen(data.path - 1)] == '/')) {
        data.path = STATIC_PATH "/index.html";
    }

    file_type_t type = get_file_info(data.path, &data.content_length);
    if (type == DIRECTORY) {
        *status_code = HTTP_NOT_IMPLEMENTED;
        goto response;
    } else if (type != REGULAR) {
        *status_code = HTTP_NOT_FOUND;
        goto response;
    }

    if (detect_content_type(data.path, &data.content_type) != EXIT_SUCCESS) {
        *status_code = HTTP_NOT_IMPLEMENTED;
    }

response:
    return make_response(data, response);
}
