#include "socklib.h"
#include "util.h"
int make_server_socket_q(int portnum,int backlog){
    struct sockaddr_in saddr;
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
        //fprintf(stdout,"hp NULL\n");
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
    return sock_id;
}

int make_server_socket(int portnum){
    return make_server_socket_q(portnum,BACKLOG);
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
