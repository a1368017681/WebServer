#include "http_response.h"
#include "debug.h"

void do_request(void* ptr) {
	http_request_t *request = (http_request_t* )ptr;
	int fd = request->fd;
	DEBUG("request fd : %d",fd);
	FILE* fp = fdopen(fd,"w");
    	fprintf(fp,"HTTP/1.0 404 Not found\r\n");
    	fprintf(fp,"Content-type: text/plain\r\n");
    	fprintf(fp, "Content-length: 8192\r\n");
    	fprintf(fp,"\r\n");

    	fprintf(fp,"The item you requested: \\ is not found\r\n");
    	fclose(fp);
}