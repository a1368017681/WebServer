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
void cannot(int);
void do_404(char *,int);
void isadir(char *);
void not_exist(char *);
void do_ls(char *,int);
char* file_type(char *);
void ends_in_cgi(char *);
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

void read_til_crnl(FILE * fp){

}

void process_rq(char *rq,int fd){

}
