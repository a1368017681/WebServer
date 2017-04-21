#ifndef _HTTP_PARSE_H_
#define _HTTP_PARSE_H_

#include <time.h>
#include <stdlib.h> 
#include "http_request.h"

typedef enum{
	HTTP_PARSE_OK = 0,
	HTTP_CONTINUE_PARSE,
	HTTP_PARSE_REQUEST_HEADER_OK,
	HTTP_PARSE_REQUEST_HEADER_FAIL,
	HTTP_PARSE_REQUEST_BODY_OK,
	HTTP_PARSE_REQUEST_BODY_FAIL,
	HTTP_PARSE_INVALID_METHOD,
	HTTP_PARSE_INVALID_REQUEST,
	HTTP_PARSE_INVALID_HEADER,
	HTTP_PARSE_INTERNAL_ERROR
}HTTP_PARSE_RESULT;

HTTP_PARSE_RESULT http_parse_request_header(http_request_t *rq);
HTTP_PARSE_RESULT http_parse_request_body(http_request_t *rq);

#endif