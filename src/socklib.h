#ifndef _SOCKLIB_H_
#define _SOCKLIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define HOSTLEN 256
#define BACKLOG 1024
#define LOCALHOST "127.0.0.1"

int make_server_socket(uint port_num);
int connect_to_server(char*,int);
int make_socket_non_blocking(int listen_fd);
void write_to_fd(int fd,char* data,int len);

#endif
