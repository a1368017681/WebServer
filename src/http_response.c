#include "http_response.h"
#include "debug.h"
#include "memory_pool.h"
#include "timer.h"
#include "socklib.h"
#include "util.h"
#include "rio.h"
#include "epoll.h"
#include "list.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>


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
static void uri_parse(char* uri,uint uri_len,char* file_name,char* query_string);
static char* ROOT = NULL;

void do_request(void* ptr) {
    http_request_t *request = (http_request_t* )ptr;
    int fd = request->fd;
    char *tmp_buf = NULL;
    del_timer(request);
    ROOT = request->root;
    LOG_INFO("do_request: ROOT = %s ,root = %s",ROOT,request->root);
    for(;;) { //读取数据流
        tmp_buf = &request->buf[request->last % MAX_BUF];
        int rest_size = MIN(MAX_BUF-(request->last - request->cur_pos) - 1, MAX_BUF-(request->last % MAX_BUF));
        int n = read(fd,tmp_buf,rest_size);
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
        request->last += n;
        LOG_INFO("ready to parse request line!%s","");
        //LOG_INFO("request-data is :\n%s",tmp_buf);

        HTTP_PARSE_RESULT ret = http_parse_request_header(request);
        if(ret != HTTP_PARSE_OK){
            if(ret != HTTP_CONTINUE_PARSE) {
                LOG_ERROR("do_request: http_parse_request_header error!%s","");
                ret = http_close_connection(request);
                CHECK(0 == ret,"do_request : http_close_connection error!%s","");
                return ;
            }
            continue;
        }
        LOG_INFO("method=%.*s",(int)(request->method_end - request->request_start),(char*)request->request_start);
        LOG_INFO("uri=%.*s",(int)(request->uri_end - request->uri_start),(char*)request->uri_start);

        ret = http_parse_request_body(request); 
        if(ret != HTTP_PARSE_OK){
            if(ret != HTTP_CONTINUE_PARSE) {
                LOG_ERROR("do_request: http_parse_request_body error!%s","");
                ret = http_close_connection(request);
                CHECK(0 == ret,"do_request : http_close_connection error!%s","");
                return ;
            }
            continue;
        }
        //http_parse_entity_body 
        /*list_t* pos;
        http_header_t* tmp;
        LIST_FOR_EACH(pos,&(request->list)) {
            tmp = LIST_ENTRY(pos,http_header_t,list);
            fprintf(stderr,"%.*s : %.*s\n",(int)(tmp->key_end - tmp->key_start),tmp->key_start,(int)(tmp->value_end - tmp->value_start),tmp->value_start);
        }
        fprintf(stderr, "%.*s\n",10 ,&request->buf[request->cur_pos]);*/
        
        http_response_t *rp = (http_response_t*)s_malloc(sizeof(http_response_t));
        CHECK_EXIT(NULL != rp,"do_request : http_response malloc error!%s","");

        init_response(rp,fd);

        char file_name[LEN];
        char query_string[LEN];
        uri_parse((char*)request->uri_start,(int)(request->uri_end - request->uri_start),file_name,query_string);
        struct stat st_buf;
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

static void uri_parse(char* uri,uint uri_len,char* file_name,char* query_string) {
    //fprintf(stderr, "uri = %.*s\n",uri_len,uri);
    CHECK(NULL != uri,"uri_parse: uri is NULL%s","");
    CHECK(uri_len < URL_MAX_LEN,"uri_parse: uri too long!%s","");
    int file_length = 0;
    uri[uri_len] = '\0';
    char* ptr = strchr(uri,QUERY_SYMBOL);
    if(ptr){
        file_length = (int)(ptr-uri);
    } else{
        file_length = uri_len;
    }
    strcpy(file_name,ROOT);
    //fprintf(stderr, "%s %s\n",file_name,ROOT);
    strncat(file_name,uri,file_length);
    char* last_sprit = strchr(file_name,'/');
    char* last_dot = strchr(last_sprit,'.');
    if(file_name[strlen(file_name) - 1] != '/' && last_dot == NULL) {
        strncat(file_name,"/",1);
    }
    if(file_name[strlen(file_name) - 1] == '/'){
        strcat(file_name,"index.html");
    }
    LOG_INFO("uri_parse: file_name = %s",file_name);
    return ;
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