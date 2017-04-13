#include "util.h"

int read_conf_file(char *file_name,server_conf_t *conf,char *buf,int len){
	return READ_CONF_OK;
}

int is_directory(const char* dirName){
	struct stat sDir;
    if(stat(dirName,&sDir) < 0)
        return 0;
    if(S_IFDIR == (sDir.st_mode & S_IFMT))
        return 1;
    return 0;
}