#include "http_header_handler.h"
#include "debug.h"

#include <time.h>
#include <stdlib.h>

static int http_header_handle_connection(http_request_t *rq,http_response_t *rp,char* field_val,uint len);
static int http_header_handle_IMS(http_request_t *rq,http_response_t *rp,char* field_val,uint len);

http_header_handler_distributor_t http_header_handler_distributor[] = {
	{"Connection",http_header_handle_connection},
	{"If-Modified-Since",http_header_handle_IMS}
};

static int http_header_handle_connection(http_request_t *rq,http_response_t *rp,char* field_val,uint len) {
	return 0;
}

static int http_header_handle_IMS(http_request_t *rq,http_response_t *rp,char* field_val,uint len) {
	return 0;
}

