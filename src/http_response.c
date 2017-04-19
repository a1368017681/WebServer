#include "http_response.h"
#include "debug.h"

mime_type_t mime[] = {
    {".html","text/html"},
    {".css","text/css"},
    {".xml","text/xml"},
    {".txt","text/plain"},
    {".png","image/png"},
    {".jpg","image/jpeg"},
    {".jpeg","image/jpeg"},
    {".gif","image/gif"},
    {".au","audio/basic"},
    {".mpg","video/mepg"},
    {".mpeg","video/mpeg"},
    {".avi","video/x-msvideo"},
    {".xhtml","application/xhtml"},
    {".pdf","application/pdf"},
    {".gz","application/x-gzip"},
    {".tar","application/x-tar"},
    {NULL,"text/plain"}
};

static char* ROOT = NULL;

void do_request(void* ptr) {
    http_request_t *request = (http_request_t* )ptr;
    int fd = request->fd;
    char file_name[LEN];
    char *tmp_buf = NULL;
    del_timer(request);
    for(;;) {
        tmp_buf = &request->buf[request->cur_pos % MAX_BUF];
        /*uint remain_size = MIN(MAX)*/
    }
    printf("do_request over\n");
}