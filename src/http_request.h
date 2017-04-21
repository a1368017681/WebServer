#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <stdlib.h> 
#include "util.h"
#include "list.h"

#define MAX_BUF 8192

/*定义所支持的HTTP请求方式*/
typedef enum {
	HTTP_METHOD_GET = 0,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST,
	HTTP_METHOD_UNKNOW
}HTTP_METHOD; 

typedef enum {
	INIT_REQUEST_OK = 0,
	INIT_REQUEST_FAIL,
	FREE_REQUEST_OK,
	FREE_REQUEST_FAIL
}HTTP_REQUEST_STATUS;

typedef struct {
	void* root;/*根目录位置*/
	int fd,epfd;
	int state; /*parser的状态机需要*/
	char buf[MAX_BUF];
	void* request_start;
	void* request_end;
	void* method_end;
	void* uri_start;
	void* uri_end;
	int major;
	int minor;
	void *timer;	
	HTTP_METHOD method;
	list_t list;

	uint cur_pos,last; /*socket读取位置*/
}http_request_t;

typedef struct {
	int fd;
	time_t modified_time;
	int modified;
	int keep_alive;
	int status;
	void* entity;
}http_response_t;

HTTP_REQUEST_STATUS init_http_request(http_request_t* request,int listed_fd,int epfd,server_conf_t* conf);
HTTP_REQUEST_STATUS free_http_request(http_request_t* request);
int http_close_connection(http_request_t* request);

#endif
