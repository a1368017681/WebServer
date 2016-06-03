#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "socklib.c"

void read_til_crnl(FILE*);
void process_rq(char *,int);
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

int main(int ac,char *av[]){
    int sock,fd;
    FILE *fpin;
    char request[BUFSIZ];
    if( ac == 1 ){
        fprintf(stderr,"usage: ws portnum\n");
        exit(1);
    }
    sock = make_server_socket(atoi(av[1]));
    if( sock == -1 ) exit(1);

    while(1){
        fd = accept(sock,NULL,NULL);
        fpin = fdopen(fd,"r");

        fgets(request,BUFSIZ,fpin);
        printf("got a call: request = %s",request);
        read_til_crnl(fpin);

        process_rq(request,fd);
        fclose(fpin);
    }
    return 0;
}

void read_til_crnl(FILE * fp){  /*handle the blank line "\r\n" */
    char buf[BUFSIZ];
    while( fgets(buf,BUFSIZ,fp) != NULL && strcmp(buf,"\r\n")!=0 );
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

void do_ls(char* dir,int fd){
    FILE* fp;

    fp = fdopen(fd,"w");
    header(fp,"text/plain");
    fprintf(fp,"\r\n");
    fflush(fp);

    dup2(fd,1);
    dup2(fd,2);
    close(fd);
    
    execlp("ls","ls","-l",dir,NULL);
    perror(dir);
    exit(1);
}

char* file_type(char* f){
    char *cp;
    if( (cp = strrchr(f,'.')) != NULL ) return cp+1;
    return "";
}

int ends_in_cgi(char* f){
    return ( strcmp(file_type(f),"cgi") == 0 );
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

