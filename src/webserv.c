#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include "socklib.h"
#include "util.h"
#include "debug.h"
#include "epoll.h"
#include "http_response.h"
#include "timer.h"

extern int errno;
extern struct epoll_event *events;

static const struct option long_options[]={
    {"help",no_argument,NULL,'?'},
    {"version",no_argument,NULL,'V'},
    {"conf",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};

static void manual(){
    fprintf(stderr, "%s", MANUAL);
}

void read_til_crnl(FILE*);
void process_rq(char *,int);
void child_waiter(int);
void header(FILE *,char *);
void cannot_do(int);
void do_404(char *,int);
int isadir(char *);
int not_exist(char *);
void do_ls(char *,int);
char* file_type(char *);
int ends_in_cgi(char *);
void do_exec(char *,int);
void do_cat(char *,int);
int IsDirectory(const char*);

int main(int ac,char *av[]){
    int opt = 0;
    int opt_index = 0;
    char *conf_file = CONF_FILE;

    if(ac <= 1) {
        manual();
        return 0;
    }

    /*use get_optlong*/
    while ((opt=getopt_long(ac, av,"Vc:?h",long_options,&opt_index)) != EOF) {
        switch (opt) {
            case 'c':
                conf_file = optarg;
                break;
            case 'V':
                fprintf(stdout,PROGRAM_VERSION"\n");
                return 0;
            case ':':
            case 'h':
            case '?':
                manual();
                return 0;
            default:
                manual();
                return 0;
        }
    }

    DEBUG("conf file = %s",conf_file);
    
    if (optind < ac) {
        LOG_ERROR("non-option ARGV-elements: %s","");
        while (optind < ac)
            LOG_ERROR("%s ", av[optind++]);
        return 0;
    }

    /*读取配置文件*/
    READ_CONF_RET rc_ret = READ_CONF_OK;
    server_conf_t conf;
    rc_ret = read_conf_file(conf_file,&conf);
    CHECK_EXIT(READ_CONF_OK == rc_ret,"read conf file error!%s","");

    /*注册signal的处理函数，解决SIGPIP可能导致的系统崩溃问题*/
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    CHECK_EXIT((0 == sigaction(SIGPIPE, &sa, NULL)),"install sigal handler for SIGPIPE failed!%s","");
    /*if (sigaction(SIGPIPE, &sa, NULL)) {
        LOG_ERROR("install sigal handler for %s failed","SIGPIPE");
        return 0;
    }*/

    /*初始化socke操作*/
    int listen_fd;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0 , sizeof(client_addr));
    socklen_t accept_len = sizeof(struct sockaddr_in);

    listen_fd = make_server_socket(conf.port);
    int ret = make_socket_non_blocking(listen_fd);
    CHECK(ret == 0, "make_socket_non_blocking error%s","");

    /*使用epoll*/
    struct epoll_event event;
    int epfd = server_epoll_create(0);

    http_request_t *http_request = (http_request_t *)malloc(sizeof(http_request_t));
    init_http_request(http_request,listen_fd,epfd,&conf);

    event.data.ptr = (void*)http_request;
    event.events = EPOLLIN | EPOLLET;
    server_epoll_add(epfd,listen_fd,&event);
    
    init_timer();
    time_t time_now = time(NULL);
    LOG_INFO("listen_fd is : %d",listen_fd);
    LOG_INFO("server start at :%s",ctime(&time_now));
    

    /*event-driven大体框架*/
    for(;;){
        int wait_time = find_timer();
        DEBUG("main wait_time is = %d",wait_time);
        int n = server_epoll_wait(epfd,events,MAXEVENTS,wait_time);
        handle_expire_timers();

        for(int i = 0; i < n; i++) {
            http_request_t *request = (http_request_t *)events[i].data.ptr;
            int fd = request->fd;

            if(fd == listen_fd) { //fd为socket建立的fd表明这是一条新的连接
                int accept_fd;

                for(;;) { //监听请求连接的数据
                    accept_fd = accept(listen_fd,(struct sockaddr_in *)&client_addr,&accept_len);
                    if(accept_fd < 0) {  //accept出错
                        if(errno != EAGAIN && errno != EWOULDBLOCK) {
                            LOG_ERROR(ACCEPT_ERROR,"");
                            break;
                        }
                        break;
                    }

                    /*为new出来的fd做socket操作*/
                    int tmp_ret = make_socket_non_blocking(accept_fd);
                    CHECK(tmp_ret == 0,"make_socket_non_blocking error fd : %d",tmp_ret);
                    LOG_INFO("new http connection fd %d",accept_fd);

                    /*初始化新连接请求*/
                    http_request_t *request_new = (http_request_t *)malloc(sizeof(http_request_t));
                    CHECK_BREAK(NULL != request_new,"malloc(http_request_t) error!%s","");
                    init_http_request(request_new,accept_fd,epfd,&conf);
                    event.data.ptr = (void *)request_new;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    /*epoll操作*/
                    server_epoll_add(epfd,accept_fd,&event);
                    add_timer(request_new,TIMEOUT_DEFAULT,http_close_connection);
                }
            } else {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                    LOG_ERROR("epoll error fd: %d", request->fd);
                    close(fd);
                    continue;
                }

                LOG_INFO("new data from fd : %d",fd);
                do_request(events[i].data.ptr);
            }
        }
    }

    /*由于TCP/IP协议栈是维护着一个接收和发送缓冲区的。
    在接收到来自客户端的数据包后，服务器端的TCP/IP协议栈应该会做如下处理：
    如果收到的是请求连接的数据包，则传给监听着连接请求端口的socetfd套接字，
    进行accept处理；如果是已经建立过连接后的客户端数据包，则将数据放入接收缓冲区。
    这样，当服务器端需要读取指定客户端的数据时，
    则可以利用socketfd_new 套接字通过recv或者read函数到缓冲区里面去取指定的数据
    （因为socketfd_new代表的socket对象记录了客户端IP和端口，因此可以鉴别）。*/
    /*FILE *fpin;
    char request[BUFSIZ];
    if( ac == 1 ){
        ERROR_STR(NO_PORTNUM_ERROR);
        exit(1);
    }
    
    signal(SIGCHLD,child_waiter);   //这里要再好好想下
    sock = make_server_socket(atoi(av[1]));
    if( sock == -1 ) exit(1);

    while(1){
        fd = accept(sock,NULL,NULL);
        if(fd == -1){ 
            if(errno != EINTR){
                ERROR_STR(ACCEPT_ERROR);
            }
            continue;
        }
        fpin = fdopen(fd,"r");

        fgets(request,BUFSIZ,fpin);
        printf("got a call: request = %s",request);
        read_til_crnl(fpin);

        process_rq(request,fd);
        fclose(fpin);
    }*/
    return 0;
}

void read_til_crnl(FILE * fp){  /*handle the blank line "\r\n" */
    char buf[BUFSIZ];
    while( fgets(buf,BUFSIZ,fp) != NULL && strcmp(buf,"\r\n")!=0 );
}

void child_waiter(int signum){
    while(waitpid(-1,NULL,WNOHANG) > 0){
        printf("wait one childpro\n");
    }
}

void process_rq(char *rq,int fd){
    char cmd[BUFSIZ],arg[BUFSIZ];
    if(fork() != 0) return ;
    strcpy(arg,".");
    if(sscanf(rq,"%s %s",cmd,arg+1) != 2) 
        return ;
    if(strcmp(cmd,"GET") != 0) 
        cannot_do(fd);
    else if(not_exist(arg)) 
        do_404(arg,fd);
    else if(isadir(arg)) 
        do_ls(arg,fd);
    else if(ends_in_cgi(arg))
        do_exec(arg,fd);
    else 
        do_cat(arg,fd);
}

void cannot_do(int fd){
    FILE* fp = fdopen(fd,"w");
    fprintf(fp,"HTTP/1.0 501 Not Implemented\r\n");
    fprintf(fp,"Content-type:text/plain\r\n");
    fprintf(fp,"\r\n");

    fprintf(fp,"That command is not yet implemented\r\n");
    fclose(fp);
}

void do_404(char *item,int fd){
    FILE* fp = fdopen(fd,"w");
    fprintf(fp,"HTTP/1.0 404 Not found\r\n");
    fprintf(fp,"Content-type:text/plain\r\n");
    fprintf(fp,"\r\n");

    fprintf(fp,"The item you requested: %s\r\nis not found\r\n",item);
    fclose(fp);
}

int isadir(char* f){
    struct stat info;
    return ( stat(f,&info) != -1 && S_ISDIR(info.st_mode) );
}

void header(FILE* fp,char* content_type){
    fprintf(fp,"HTTP/1.0 200OK\r\n");
    if( content_type ) 
        fprintf(fp,"Content-type: %s\r\n",content_type);
}

int not_exist(char* f){
    struct stat info;
    return ( stat(f,&info) == -1 );
}

int IsDirectory(const char* dirName){
    struct stat sDir;
    if(stat(dirName,&sDir) < 0)
        return 0;
    if(S_IFDIR == (sDir.st_mode & S_IFMT))
        return 1;
    return 0;
}

void do_ls(char* dir,int fd){
    FILE* fp;
    DIR *dp;
    struct dirent *dirp;
    char filePath[BUFSIZ];
    char fileName[BUFSIZ];

    if(strcmp(dir,".") == 0)
        strcat(dir,"/");
    fprintf(stdout, "dirname: %s\n", dir);
    if((dp = opendir(dir)) == NULL)
        ERROR_STR("open dir error");
    fp = fdopen(fd,"w");
    header(fp,"text/html");

    fprintf(fp,"\r\n");
    fprintf(fp, "<font size=\"20\" color=\"blue\">Index of /</font></br></br>\r\n");
    while((dirp = readdir(dp)) != NULL){
        strcpy(filePath,"");
        strcpy(fileName,"");
        strcat(filePath,dir);
        strcat(filePath,dirp->d_name);
        strcat(fileName,dirp->d_name);
        if(IsDirectory(fileName) && strcmp(fileName,"./") && strcmp(fileName,"../"))
        	strcat(fileName,"/");
        fprintf(stdout, "%s\n",filePath);
        if(!strcmp(fileName,"./"))
        	continue;
        if(IsDirectory(filePath) && strcmp(filePath,"./") && strcmp(filePath,"../"))
        	fprintf(fp, "<a href=\"./%s\">%s/</a></br>\r\n", fileName,dirp->d_name);
        else
       		fprintf(fp, "<a href=\"./%s\">%s</a></br>\r\n",  fileName,dirp->d_name);
    }
    fflush(fp);

    dup2(fd,1);
    dup2(fd,2);
    close(fd);
    
}

char* file_type(char* f){
    char *cp;
    if( (cp = strrchr(f,'.')) != NULL ) return cp+1;
    return "";
}

int ends_in_cgi(char* f){
    return ( strcmp(file_type(f),"cgi") == 0 || strcmp(file_type(f),"rb") == 0 );
}

void do_exec(char* prog,int fd){
    FILE* fp;

    fp = fdopen(fd,"w");
    header(fp,NULL);
    fflush(fp);
    dup2(fd,1);
    dup2(fd,2);
    close(fd);
    execl(prog,prog,NULL);
    perror(prog);
}

void do_cat(char* f,int fd){
    char* extension = file_type(f);
    char* content = "text/plain";
    FILE *fpsock,*fpfile;
    int c;

    if( strcmp(extension,"html") == 0 )
        content = "text/html";
    else if( strcmp(extension,"gif") == 0 )
        content = "image/gif";
    else if( strcmp(extension,"jpg") == 0 )
        content = "image/jpeg";
    else if( strcmp(extension,"jpeg") == 0 )
        content = "image/jpeg";

    fpsock = fdopen(fd,"w");
    fpfile = fopen(f,"r");
    if( fpsock != NULL && fpfile != NULL){
        header(fpsock,content);
        fprintf(fpsock,"\r\n");
        while( (c = getc(fpfile)) != EOF)
            putc(c,fpsock);
        fclose(fpfile);
        fclose(fpsock);
    }
    exit(0);
}

