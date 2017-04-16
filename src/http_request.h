#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <time.h>
#include <stdlib.h> 
#include "http_response.h"
#include "util.h"

#define MAX_BUF 8192

/*定义所支持的HTTP请求方式*/
typedef enum {
	HTTP_METHOD_GET = 0,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST,
	HTTP_METHOD_UNKNOW
}HTTP_METHOD; 

typedef struct {
	void* root;//根目录位置
	int fd,epfd;
	char buf[MAX_BUF];
	int major;
	int minjor;		
}http_request_t;

void init_http_request(http_request_t* request,int listed_fd,int epfd,server_conf_t* conf);

#endif
