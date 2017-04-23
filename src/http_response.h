#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <time.h>
#include "http_request.h"
#include "http_parse.h"
#include "http_header_handler.h"

#define LEN 1059
#define QUERY_SYMBOL '?'

typedef enum {
	INIT_RESPONSE_OK = 0,
	INIT_RESPONSE_FAIL
}HTTP_RESPONSE_STATUS; 

typedef struct {
	char *file_type;
	char *file_val;
}mime_type_t;

typedef struct {
	HTTP_STATUS_CODE code;
	char* description;
}http_status_code_description_t;

HTTP_RESPONSE_STATUS init_response(http_response_t *rp,int fd);
void do_request(void* request);

#endif
