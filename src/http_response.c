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
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


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
static void process_response(int fd, char *file_name, size_t file_size, http_response_t *rp);
static const char* get_file_type(const char* file);
static char* ROOT = NULL;

void do_request(void* ptr) {
    http_request_t *request = (http_request_t* )ptr;
    int fd = request->fd;
    char *tmp_buf = NULL;
    del_timer(request);
    ROOT = request->root;
    //LOG_INFO("do_request: ROOT = %s ,root = %s",ROOT,request->root);
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

        int ret = http_parse_request_header(request);
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
        /*fprintf(stderr, "%.*s\n",10 ,&request->buf[request->cur_pos]);*/
        
        http_response_t *rp = (http_response_t*)s_malloc(sizeof(http_response_t));
        CHECK_EXIT(NULL != rp,"do_request : http_response malloc error!%s","");

        init_response(rp,fd);

        char file_name[LEN];
        char query_string[LEN];
        uri_parse((char*)request->uri_start,(int)(request->uri_end - request->uri_start),file_name,query_string);
        LOG_INFO("do_request: file_name is %s",file_name);
        struct stat st_buf;
        //struct stat tt;
        //fprintf(stderr, "do_request: file_name: %s stat: %d\n", file_name, stat(file_name,&tt));
        ret = stat(file_name,&st_buf);
        if(ret < 0) {
            if(errno == ENOENT) {
                do_response_by_code(fd,file_name,HTTP_NOT_FOUND,get_description_by_code(HTTP_NOT_FOUND),"server cannot find file!");
            }else if(errno == EACCES) {
                //LOG_INFO("asdfsadfgdfg%s","");
                do_response_by_code(fd,file_name,HTTP_FORBIDDEN,get_description_by_code(HTTP_FORBIDDEN),"file forbidden access!");
            }else {
                do_response_by_code(fd,file_name,HTTP_INTERNAL_SERVER_ERROR,get_description_by_code(HTTP_INTERNAL_SERVER_ERROR),"server internal error!");
            }
            //LOG_INFO("stat error!%s","");
            continue;
        }
        if(!(S_ISREG(st_buf.st_mode)) || !(S_IRUSR & st_buf.st_mode)){
            //LOG_INFO("asdas%s","");
            do_response_by_code(fd,file_name,HTTP_FORBIDDEN,get_description_by_code(HTTP_FORBIDDEN),"file_forbidden access!");
            continue;
        }
        rp->modified_time = st_buf.st_mtime;
        handle_header_handler(request,rp);
        CHECK(list_empty(&(request->list)) == 1,"do_request: after handle header,list should be empty!%s","");
        if(!(rp->status)){
            rp->status = HTTP_OK;
        }

        process_response(fd,file_name,st_buf.st_size,rp);
        //do_response_by_code(fd,file_name,HTTP_OK,get_description_by_code(HTTP_OK),"OK");
        if(!rp->keep_alive) {
            s_free(rp);
            ret = http_close_connection(request);
            CHECK(0 == ret,"do_request : http_close_connection error!%s","");
            return ;
        }
        s_free(rp);
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
    //LOG_INFO("uri_parse: file_name = %s",file_name);
    return ;
}

static void process_response(int fd, char *file_name, size_t file_size, http_response_t *rp) {
    char header[MAX_BUF];
    char tmp_buf[MAX_BUF];
    sprintf(header,"HTTP/1.1 %d %s\r\n",rp->status,get_description_by_code(rp->status));
    if(rp->keep_alive) {
        sprintf(header,"%sConnection: keep-alive\r\n",header);
        sprintf(header,"%sKeep-Alive: timeout=%d\r\n",header,TIMEOUT_DEFAULT);
    }
    const char* last_dot = strrchr(file_name,'.');

    const char* file_type = get_file_type(last_dot);
    LOG_INFO("file_type is %s",last_dot);
    struct tm time;
    if(rp->modified) {
        sprintf(header, "%sContent-type: %s\r\n", header, file_type);
        sprintf(header, "%sContent-length: %zu\r\n", header, file_size);
        localtime_r(&(rp->modified_time), &time);
        strftime(tmp_buf, LEN,  "%a, %d %b %Y %H:%M:%S GMT", &time);
        sprintf(header, "%sLast-Modified: %s\r\n", header, tmp_buf);
    }
    sprintf(header, "%sServer: WebServer\r\n\r\n", header); 
    
    //do_response_by_code(fd,file_name,HTTP_OK,get_description_by_code(HTTP_OK),"OK");
    size_t n = (size_t) rio_writen(fd,header,strlen(header));
    if(n != strlen(header)) {
        LOG_ERROR("process_response: rio wriren error!%s","");
        return ;
    }
    if(rp->modified) {
        int srcfd = open(file_name, O_RDONLY, 0);
        CHECK(srcfd > 2, "process_response: open error!%s","");
        // can use sendfile
        char *srcaddr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
        CHECK(srcaddr != (void *) -1, "process_response: mmap error!%s","");
        close(srcfd);
        n = rio_writen(fd, srcaddr, file_size);
        // check(n == filesize, "rio_writen error");
        munmap(srcaddr, file_size);
    }
    return ;
}

static void do_response_by_code(int fd,char* entity,HTTP_STATUS_CODE code,const char* code_description,char* msg_entity) {
    if(NULL == code_description) {
        LOG_ERROR("do_response_by_code : code_description = NULL!%s","");
        return ;
    }
    char header[MAX_BUF],body[MAX_BUF];

    sprintf(body, "<html><body>%s</body></html>",msg_entity);
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

static const char* get_file_type(const char* file_type) {
    int i;
    if(file_type == NULL) {
        return "text/plain";
    }
    int size = sizeof(mime)/sizeof(mime_type_t);
    for(i = 0; i < size; i++) {
        if(strcmp(file_type,mime[i].file_type) == 0) {
            return mime[i].file_val;
        }
    }
    return mime[i-1].file_val;
}