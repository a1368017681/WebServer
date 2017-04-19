#include "http_request.h"
#include <unistd.h>
/*初始化http_request*/
HTTP_REQUEST_STATUS init_http_request(http_request_t* request,int listed_fd,int epfd,server_conf_t* conf) {
	request->fd = listed_fd;
	request->epfd = epfd;
	request->root = conf->root;
	request->cur_pos = 0;
	request->last = 0;
	LIST_INIT_HEAD(&(request->header_list));
	return INIT_REQUEST_OK;
}

HTTP_REQUEST_STATUS free_http_request(http_request_t* request) {
	return FREE_REQUEST_OK;
}

int http_close_connection(http_request_t* request) {
	close(request->fd);
	DEBUG("fd %d closed!",request->fd);
	free(request);
	return 0;
}