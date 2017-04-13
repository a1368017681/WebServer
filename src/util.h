#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define CONF_FILE "server.conf"
#define PROGRAM_VERSION "1.0"

#define BUF_LEN 1024

#define NO_PORTNUM_ERROR "no portnum error!"
#define SOCKET_ERROR "build socket failed!"
#define CANNOT_FIND_HOST "cannot find host by name(maybe gethostbyname() error)! use 127.0.0.1 directly"
#define BIND_ERROR "bind port error!"
#define LISTEN_ERROR "listen port error!"
#define ACCEPT_ERROR "socket accept error!"

#define HTML_TEXT "<html><head><title></title></head><body></body></html>"

#define CUR_DIR "."
#define FATHER_DIR ".."

#define MANUAL "zaver [option]... \n \
	  -c|--conf <config file>  Specify config file. Default ./server.conf.\n \
	  -?|-h|--help             This information.\n \
	  -V|--version             Display program version.\n"

#define ERROR_INFO(str)\
	fprintf(stderr, "%s:%d: %s\n", __FILE__,__LINE__,str);\
	perror("");

#define ERROR_STR(str) \
	fprintf(stderr, "%s:%d: %s\n", __FILE__,__LINE__,str);

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef enum
{
	READ_CONF_OK = 0,
	READ_CONF_FAIL
}READ_CONF_RET;

typedef struct{
	void* root;
	int port;
	int thread_num;
}server_conf_t;

int read_conf_file(char *file_name,server_conf_t *conf,char *buf,int len);
int is_directory(const char* dirName);

#endif
