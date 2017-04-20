#include "socklib.h"
#include "util.h"
#include "debug.h"

int make_server_socket(uint port_num) {
    CHECK_EXIT((port_num <= 65535 && port_num > 0),"port_num invalid%s","");
    
    int listen_fd,opt_val = 1;
    struct sockaddr_in server_addr;
    /*socket操作*/
    if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        LOG_ERROR(SOCKET_ERROR,"");
        return -1;
    }
    /*一般来说 ， 一个端口释放后会等待两分钟之后才能再被使用 ， SO_REUSEADDR 是让端口释放后立即就可以被再次使用。*/    
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&opt_val , sizeof(int)) < 0) {
        LOG_ERROR(SET_SOCKET_OPT_ERROR,"");
        return -1;
    }

    bzero((void*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_num);

    if( bind(listen_fd,(struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR(BIND_ERROR,"");
        return -1;
    }

    if( listen(listen_fd,BACKLOG) < 0) {
        LOG_ERROR(LISTEN_ERROR,"");
        return -1;
    }

    return listen_fd;
    /*struct sockaddr_in saddr;
    struct hostent* hp;
    char hostname[HOSTLEN];
    int sock_id;

    sock_id = socket(PF_INET,SOCK_STREAM,0);
    if(sock_id == -1) {
        ERROR_INFO(SOCKET_ERROR);
        return -1;
    }
    bzero((void *)&saddr,sizeof(saddr));
    gethostname(hostname,HOSTLEN);
    hp = gethostbyname(hostname);
    if(hp == NULL){
        saddr.sin_addr.s_addr = inet_addr(LOCALHOST);
        ERROR_INFO(CANNOT_FIND_HOST);

    }else{
        bcopy((void *)hp->h_addr,(void *)&saddr.sin_addr,hp->h_length);
        //fprintf(stdout, "asdasdasd\n");
    }
    saddr.sin_port = htons(portnum);
    saddr.sin_family = AF_INET;
    if( bind(sock_id,(struct sockaddr *)&saddr,sizeof(saddr)) != 0){
        ERROR_INFO(BIND_ERROR);
        return -1;
    }
    if( listen(sock_id,backlog) != 0 ) {
        ERROR_INFO(LISTEN_ERROR);
        return -1;
    }
    return sock_id;*/
}

int make_socket_non_blocking(int listen_fd){
    int flags;
    int flags_later;
    flags = fcntl(listen_fd,F_GETFL,0);
    if(flags == -1){
        LOG_ERROR("fcntl error before nonblock, ret_val not expect -1%s","");
        return -1;
    }

    flags |= O_NONBLOCK;
    flags_later = fcntl(listen_fd,F_SETFL,flags);
    if(flags_later == -1){
        LOG_ERROR("fcntl error afternonblock, ret_val not expect -1%s","");
        return -1;
    } 

    return 0;
}

int connect_to_server(char *host,int portnum){
    int sock;
    struct sockaddr_in servadd;
    struct hostent* hp;

    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock == -1) return -1;
    bzero(&servadd,sizeof(servadd));
    hp = gethostbyname(host);
    if( hp == NULL ) return -1;
    bcopy(hp->h_addr,(struct sockaddr *)&servadd.sin_addr,hp->h_length);
    servadd.sin_port = htons(portnum);
    servadd.sin_family = AF_INET;

    if( connect(sock,(struct sockaddr *)&servadd,sizeof(servadd)) != 0)
        return -1;
    return sock;
}

void write_to_fd(int fd,char* data,int len) {
    return ;
}