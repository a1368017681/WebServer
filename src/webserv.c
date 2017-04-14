#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include "socklib.h"
#include "util.h"
#include "debug.h"
#include "epoll.h"

extern int errno;
extern struct epoll_event *event;

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
    socklen_t inlen = 1;

    listen_fd = make_server_socket(conf.port);
    int ret = make_socket_non_blocking(listen_fd);
    CHECK(ret == 0, "make_socket_non_blocking error%s","");

    /*使用epoll*/
    struct epoll_event event;
    int epfd = server_epoll_create(0);
    
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

