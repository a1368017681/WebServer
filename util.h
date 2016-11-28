#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define NO_PORTNUM_ERROR "no portnum error!"
#define SOCKET_ERROR "build socket failed!"
#define CANNOT_FIND_HOST "cannot find host by name(maybe gethostbyname() error)! use 127.0.0.1 directly"
#define BIND_ERROR "bind port error!"
#define LISTEN_ERROR "listen port error!"
#define ACCEPT_ERROR "socket accept error!"
#define HTML_TEXT "<html><head><title></title></head><body></body></html>"

#define ERROR_INFO(str)\
	fprintf(stderr, "%s:%d: %s\n", __FILE__,__LINE__,str);\
	perror("");

#define ERROR_STR(str) \
	fprintf(stderr, "%s:%d: %s\n", __FILE__,__LINE__,str);

#endif