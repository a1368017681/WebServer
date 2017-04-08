#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "socklib.h"
#include "util.h"

extern int errno;

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
    int sock,fd;
    FILE *fpin;
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
    }
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

