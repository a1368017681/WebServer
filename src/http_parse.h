#ifndef _HTTP_PARSE_H_
#define _HTTP_PARSE_H_

#include <time.h>
#include <stdlib.h> 

typedef enum{
	HTTP_PARSE_OK = 0,
	HTTP_PARSE_INVALID_METHOD,
	HTTP_PARSE_INVALID_REQUEST,
	HTTP_PARSE_INVALID_HEADER,
	HTTP_PARSE_INTERNAL_ERROR
}HTTP_PARSE_RESULT;


#endif