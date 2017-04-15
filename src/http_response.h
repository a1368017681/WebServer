#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <time.h>
#include "http_request.h"
#include "http_parse.h"

typedef enum {
	HTTP_OK = 200,
	HTTP_NOT_MODIFIED = 304,
	HTTP_NOT_FOUND = 404
}HTTP_STATUS_CODE;
#endif