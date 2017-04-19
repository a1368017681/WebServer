#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <time.h>
#include "http_request.h"
#include "http_parse.h"
#include "http_header_handler.h"

#define LEN 4096

typedef enum {
	HTTP_OK = 200,
	HTTP_CREATED = 201,
	HTTP_ACCEPTED = 202,
	HTTP_NO_CONTENT = 204,
	HTTP_MULTIPLE_CHOICES = 300, /*不被HTTP/1.0直接使用，只作为3xx类型回应的缺省解释*/
	HTTP_MOVED_PERMANENTLY = 301,
	HTTP_MOVED_TEMPORARILY = 302,
	HTTP_NOT_MODIFIED = 304,
	HTTP_BAD_REQUEST = 400,
	HTTP_UNAUTHORIZED = 401,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_INTERNAL_SERVER_ERROR = 500,
	HTTP_NOT_IMPLEMENTED = 501,
	HTTP_BAD_GATEWAY = 502,
	HTTP_SERVICE_UNAVAILABLE = 503,
	HTTP_EXTENSION_CODE = 999
}HTTP_STATUS_CODE;

typedef struct {
	char *file_type;
	char *file_val;
}mime_type_t;


void do_request(void* request);

#endif
