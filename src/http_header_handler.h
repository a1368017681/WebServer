#ifndef _HTTP_HEADER_HANDLER_H_
#define _HTTP_HEADER_HANDLER_H_

#include "http_request.h"
#include "util.h"

typedef int (*http_header_handler_ptr)(http_request_t *rq,http_response_t *rp,char* field_val,uint len);

typedef struct {
	char* head_field_name;
	http_header_handler_ptr handler;
}http_header_handler_distributor_t;

#endif