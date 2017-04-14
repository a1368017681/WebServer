#ifndef _EPOLL_H_
#define _EPOLL_H_

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
#include <sys/epoll.h>

#define MAXEVENTS 1024

int server_epoll_create(int flags);
void server_epoll_add(int epfd,int fd,struct epoll_event* event);
void server_epoll_mod(int epfd,int fd,struct epoll_event* event);
void server_epoll_del(int epfd,int fd,struct epoll_event* event);
int server_epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout);

#endif
