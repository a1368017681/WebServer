#include "http_response.h"
#include "debug.h"
#include "memory_pool.h"
#include "timer.h"
#include "socklib.h"
#include "util.h"
#include "rio.h"
#include "epoll.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

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

http_status_code_description_t http_status_code_description[] = {
    { HTTP_OK, "OK"},
    { HTTP_CREATED, "Created"},
    { HTTP_ACCEPTED, "Accepted"},
    { HTTP_NO_CONTENT, "No Content"},
    { HTTP_MULTIPLE_CHOICES, "Multiple Choices"}, /*不被HTTP/1.0直接使用，只作为3xx类型回应的缺省解释*/
    { HTTP_MOVED_PERMANENTLY, "Moved Permanently"},
    { HTTP_MOVED_TEMPORARILY, "Moved Temporarily"},
    { HTTP_NOT_MODIFIED, "Not Modified"},
    { HTTP_BAD_REQUEST, "Bad Request"},
    { HTTP_UNAUTHORIZED, "Unauthorized"},
    { HTTP_FORBIDDEN, "Forbidden"},
    { HTTP_NOT_FOUND, "Not Found"},
    { HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    { HTTP_NOT_IMPLEMENTED, "Not Implement"},
    { HTTP_BAD_GATEWAY, "Bad Gateway"},
    { HTTP_SERVICE_UNAVAILABLE, "Service Unavailable"}
};

static void do_response_by_code(int fd,char* entity,HTTP_STATUS_CODE code,const char* code_description,char* msg_entity);
static const char* get_description_by_code(HTTP_STATUS_CODE code);
static char* ROOT = NULL;

void do_request(void* ptr) {
    http_request_t *request = (http_request_t* )ptr;
    int fd = request->fd;
    char file_name[LEN];
    char *tmp_buf = (char*)s_malloc(sizeof(char)*MAX_BUF);
    del_timer(request);
    for(;;) { //读取数据流
        //tmp_buf = &request->buf[request->cur_pos % MAX_BUF];
        int n = read(fd,tmp_buf,MAX_BUF);
        if(n == 0) {
            LOG_INFO("read call ret = 0,need to close connection!%s","");
            int ret = http_close_connection(request);
            CHECK(0 == ret,"do_request : http_close_connection error!%s","");
            return ;
        } else if( n < 0 ) { //发生错误
            if(errno == EAGAIN) {
                break;
            } else {
                if(errno == EINTR) {
                    LOG_ERROR("read call ret < 0,errno = EINTR ,maybe off the net%s","");    
                }else {
                    LOG_ERROR("read call ret < 0,errno = %d",errno);
                }      
                int ret = http_close_connection(request);
                CHECK(0 == ret,"do_request : http_close_connection error!%s","");
                return ;
            }
        }

        LOG_INFO("ready to parse request line!%s","");
        LOG_INFO("data is %s",tmp_buf);

        http_response_t *rp = (http_response_t*)s_malloc(sizeof(http_response_t));
        CHECK_EXIT(NULL != rp,"do_request : http_response malloc error!%s","");

        init_response(rp,fd);

        do_response_by_code(fd,file_name,HTTP_OK,get_description_by_code(HTTP_OK),"");
    }

    struct epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    server_epoll_mod(request->epfd, request->fd, &event);
    add_timer(request, TIMEOUT_DEFAULT, http_close_connection);
    return;
}

HTTP_RESPONSE_STATUS init_response(http_response_t* rp,int fd) {
    rp->fd = fd;
    rp->modified = 1;
    rp->keep_alive = 0;
    rp->status = 0;
    return INIT_RESPONSE_OK;  
}

static void do_response_by_code(int fd,char* entity,HTTP_STATUS_CODE code,const char* code_description,char* msg_entity) {
    if(NULL == code_description) {
        LOG_ERROR("do_response_by_code : code_description = NULL!%s","");
        return ;
    }
    char header[MAX_BUF],body[MAX_BUF];

    sprintf(body, HTML_TEXT);
    /*sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
    sprintf(body, "%s%d: %s\n", body, code, msg_entity);
    sprintf(body, "%s<p>%s: %s\n</p>", body, msg_entity, entity);
    sprintf(body, "%s<hr><em>Web server</em>\n</body></html>", body);*/

    sprintf(header,"HTTP/1.0 %d %s\r\n",code,code_description);
    sprintf(header,"%sServer: WebServer\r\n",header);
    sprintf(header,"%sContent-type: text/html\r\n",header);
    sprintf(header,"%sContent-length: %d\r\n\r\n",header,(int)strlen(body));

    rio_writen(fd,header,strlen(header));
    rio_writen(fd,body,strlen(body));
    return ;
}

static const char* get_description_by_code(HTTP_STATUS_CODE code) {
    int i;
    int size = sizeof(http_status_code_description)/sizeof(http_status_code_description_t);
    for(i = 0; i < size; i++) {
        if(code == http_status_code_description[i].code) {
            return http_status_code_description[i].description;
        }
    }
    return NULL;
}