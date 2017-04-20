#include "rio.h"
#include "debug.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


ssize_t rio_readn(int fd,void* usrbuf,size_t n) {
	size_t nleft = n;
	ssize_t nread;
	char* bufp = (char*)usrbuf;

	while(nleft > 0) {
		nread = read(fd,bufp,nleft);
		if(nread < 0) {
			if(errno == EINTR) {
				nread = 0;
			}else {
				return -1;
			}
		}else if(nread == 0) {
			break;
		}
		nleft -= nread;
		bufp += nread;
	}
	return (n-nleft);
}

ssize_t rio_writen(int fd,void* usrbuf,size_t n) {
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = (char*)usrbuf;

	while(nleft > 0) {
		nwritten = write(fd,bufp,nleft);
		if(nwritten <= 0) {
			if(errno == EINTR){
				nwritten = 0;
			}else{
				return -1;
			}
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}
