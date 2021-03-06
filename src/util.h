#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "parson.h"
#include "debug.h"

typedef unsigned int uint;
typedef unsigned long long ull;

#define CONF_FILE "server.json"
#define PROGRAM_VERSION "1.0"


#define BUF_LEN 4096

#define HTML_TEXT "<html><head><title></title></head><body>Hello World</body></html>"

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

READ_CONF_RET read_conf_file(char *file_name,server_conf_t *conf);
int is_directory(const char* dirName);

#endif
