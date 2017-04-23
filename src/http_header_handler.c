#include "http_header_handler.h"
#include "debug.h"
#include "list.h"
#include "memory_pool.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int http_header_handle_connection(http_request_t *rq,http_response_t *rp,char* field_val,uint len);
static int http_header_handle_IMS(http_request_t *rq,http_response_t *rp,char* field_val,uint len);
static int http_header_handle_ignore(http_request_t *rq,http_response_t *rp,char* field_val,uint len);

http_header_handler_distributor_t http_header_handler_distributor[] = {
	{"Connection",http_header_handle_connection},
	{"If-Modified-Since",http_header_handle_IMS},
	{"",http_header_handle_ignore}
};

static int http_header_handle_connection(http_request_t *rq,http_response_t *rp,char* field_val,uint len) {
	if(strncasecmp("keep-alive",field_val,len) == 0) {
		rp->keep_alive = 1;
	}
	return HEADER_HANDLE_OK;
}

static int http_header_handle_IMS(http_request_t *rq,http_response_t *rp,char* field_val,uint len) {
	struct tm time;
	if (strptime(field_val, "%a, %d %b %Y %H:%M:%S GMT", &time) == (char *)NULL) {
        return HEADER_HANDLE_OK;
    }
    time_t now = mktime(&time);
    if(fabs(now - rp->modified_time) < 1e-6){
    		rp->modified_time = 0;
    		rp->status = HTTP_NOT_MODIFIED;
    }
    return HEADER_HANDLE_OK;
}

static int http_header_handle_ignore(http_request_t *rq,http_response_t *rp,char* field_val,uint len) {
	return HEADER_HANDLE_OK;
}

void handle_header_handler(http_request_t *rq,http_response_t *rp) {
	http_header_t *header;
	http_header_handler_distributor_t *distributor;
	list_t *pos;
	LIST_FOR_EACH(pos, &(rq->list)) {
        header = LIST_ENTRY(pos, http_header_t, list);
        for (distributor = http_header_handler_distributor;strlen(distributor->head_field_name) > 0;
            distributor++) {
            if (strncmp(header->key_start, distributor->head_field_name, header->key_end - header->key_start) == 0) {       
                //debug("key = %.*s, value = %.*s", header->key_end-header->key_start, header->key_start, header->value_end-header->value_start, header->value_start);
                uint len = header->value_end - header->value_start;
                (*(distributor->handler))(rq, rp, header->value_start, len);
                break;
            }    
        }
        LIST_DEL_NODE(pos);
        s_free(header);
    }
}