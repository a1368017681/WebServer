#ifndef _RIO_H_
#define _RIO_H_

#include <unistd.h>
#include <sys/types.h>

/*参考CSAPP的实现*/

#define RIO_BUFSIZE 8192


ssize_t rio_readn(int fd,void* usrbuf,size_t n);
ssize_t rio_writen(int fd,void* usrbuf,size_t n);

#endif