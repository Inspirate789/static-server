#ifndef DECISIONS_MAKER_H
#define DECISIONS_MAKER_H

#include "../../inc/http/request.h"
#include "../../inc/http/response.h"

#define STATIC_PATH "/tmp/static"

int make_decision(http_request_t request, http_response_t *response, http_status_code_t *status_code);

#endif //DECISIONS_MAKER_H
