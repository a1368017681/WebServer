#include "epoll.h"
#include "debug.h"
#include "util.h"

struct epoll_event *events;

int server_epoll_create(int flags) {
	int fd = epoll_create1(flags);
	CHECK(fd > 0,"epoll_create error!%s","");

	events = (struct epoll_event *)malloc(sizeof(struct epoll_event*) * MAXEVENTS);
	CHECK(events != NULL,"epoll_create error! malloc error%s","");
	return fd;
}

void server_epoll_add(int epfd,int fd,struct epoll_event* event) {
	int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,event);
	CHECK(ret == 0,"epoll_add error!%s","");
	return ;
}

void server_epoll_mod(int epfd,int fd,struct epoll_event* event) {
	int ret = epoll_ctl(epfd,EPOLL_CTL_MOD,fd,event);
	CHECK(ret == 0,"epoll_mod error!%s","");
	return ;
}

void server_epoll_del(int epfd,int fd,struct epoll_event* event) {
	int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,fd,event);
	CHECK(ret == 0,"epoll_del error!%s","");
	return ;
}

int server_epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout){
	int n = epoll_wait(epfd,events,maxevents,timeout);
	CHECK(n >= 0,"epoll_wait error!%s","");
	return n;
}
