#include "http_request.h"

/*初始化http_request*/
void init_http_request(http_request_t* request,int listed_fd,int epfd,server_conf_t* conf) {
	request->fd = listed_fd;
	request->epfd = epfd;
	request->root = conf->root;
}