#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../inc/http/decisions_maker.h"
#include "../../lib/log/log.h"

struct decisions_maker {
};

int decisions_maker_create(decisions_maker_t *maker) {
    if (maker == NULL) {
        log_error("maker pointer is NULL");
        return EXIT_FAILURE;
    }

    decisions_maker_t tmp_handler = malloc(sizeof(struct decisions_maker));
    if (tmp_handler == NULL) {
        log_error("decisions_maker_create malloc() struct decisions_maker: %s", strerror(errno));
        return errno;
    }

    *maker = tmp_handler;

    return EXIT_SUCCESS;
}

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

static int setup_success_template(http_response_t *response, http_proto_t proto) {
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

static int setup_forbidden_template(http_response_t *response, http_proto_t proto) {
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

static int setup_not_found_template(http_response_t *response, http_proto_t proto) {
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

static int setup_not_allowed_template(http_response_t *response, http_proto_t proto) {
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

static int setup_fail_template(http_response_t *response, http_proto_t proto) {
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

int make_decision(decisions_maker_t maker, http_request_t request, http_response_t *response, http_status_code_t *status_code) { // TODO
    if (maker == NULL) {
        log_error("maker is NULL");
        return EXIT_FAILURE;
    }
    if (request == NULL) {
        log_error("request is NULL");
        return EXIT_FAILURE;
    }
    if (response == NULL) {
        log_error("response is NULL");
        return EXIT_FAILURE;
    }
    if (status_code == NULL) {
        log_error("status_code is NULL");
        return EXIT_FAILURE;
    }

    char *proto = NULL;
    int rc = http_request_get_proto(request, &proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    rc = setup_success_template(response, proto);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }
    *status_code = HTTP_OK;

    return EXIT_SUCCESS;
}

void decisions_maker_destroy(decisions_maker_t *maker) {
    if (maker == NULL || *maker == NULL) {
        return;
    }
    free(*maker);
    *maker = NULL;
}
