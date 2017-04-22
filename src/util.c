#include "util.h"
#include <string.h>

/*读取配置文件*/
READ_CONF_RET read_conf_file(char *file_name,server_conf_t *conf) {
	FILE *fp = fopen(file_name, "r");
	if(!fp){
		LOG_ERROR("open config file :%s failed!",file_name);
		return OPEN_CONF_FILE_FAIL;
	}

	JSON_Value *server_conf = json_parse_file(file_name);
	if(!server_conf) {
		LOG_ERROR("config file :%s parse failed!",file_name);
		return CONF_FILE_PARSE_FAIL;
	}

	const char* root = json_object_get_string(json_object(server_conf), "root");
	if(!root) {
		LOG_ERROR("config file :%s form wrong, root value wrong",file_name);
		return CONF_FILE_FORM_WRONG;
	}
	uint port = json_object_get_number(json_object(server_conf), "port");
	if(0 == port){
		LOG_ERROR("config file :%s form wrong, port value wrong",file_name);
		return CONF_FILE_FORM_WRONG;	
	}
	uint thread_num = json_object_get_number(json_object(server_conf), "threadnum");
	if(0 == thread_num) {
		LOG_ERROR("config file :%s form wrong, thread_num value wrong",file_name);
		return CONF_FILE_FORM_WRONG;
	}

	char* tmp = (char*)malloc(sizeof(root));
	strcpy(tmp,root);
	conf->root = (void*)tmp;
	conf->port = port;
	conf->thread_num = thread_num;

	json_value_free(server_conf);
	fclose(fp);
	return READ_CONF_OK;
}

int is_directory(const char* dir_name){
	struct stat s_dir;
    if(stat(dir_name,&s_dir) < 0)
        return 0;
    if(S_IFDIR == (s_dir.st_mode & S_IFMT))
        return 1;
    return 0;
}