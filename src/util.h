#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "parson.h"
#include "debug.h"

#define CONF_FILE "server.json"
#define PROGRAM_VERSION "1.0"

#define BUF_LEN 4096

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
	OPEN_CONF_FILE_FAIL,
	CONF_FILE_PARSE_FAIL,
	CONF_FILE_FORM_WRONG,
	READ_CONF_FAIL
}READ_CONF_RET;

typedef struct{
	void* root;
	uint port;
	uint thread_num;
}server_conf_t;

typedef unsigned int uint;
READ_CONF_RET read_conf_file(char *file_name,server_conf_t *conf);
int is_directory(const char* dirName);

#endif
