#ifndef DECISIONS_MAKER_H
#define DECISIONS_MAKER_H

#include "../../inc/http/request.h"
#include "../../inc/http/response.h"

typedef struct decisions_maker *decisions_maker_t;

int decisions_maker_create(decisions_maker_t *maker);
int make_decision(decisions_maker_t maker, http_request_t request, http_response_t *response, http_status_code_t *status_code);
void decisions_maker_destroy(decisions_maker_t *maker);

#endif //DECISIONS_MAKER_H
