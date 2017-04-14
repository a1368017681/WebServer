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
#include "debug.h"

#define HOSTLEN 256
#define BACKLOG 1
#define LOCALHOST "127.0.0.1"

int make_server_socket_q(int,int);
int make_server_socket(int);
int connect_to_server(char*,int);

#endif
