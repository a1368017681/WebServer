#include "http_parse.h"
#include "util.h"
#include "list.h"
#include "memory_pool.h"

#define CR '\r'
#define LF '\n'
#define CRLF '\r\n'
#define CRLFCRLF '\r\n\r\n'

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define LOWER(c) (unsigned char)(c | 0x20)
#define IS_LOWERCASE(c) ((c) >= 'a' && (c) <= 'z')
#define IS_UPPERCASE(c) ((c) >= 'A' && (c) <= 'Z')
#define IS_ALPHA(c) (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_NUM_GT_ZERO(c)      ((c) >= '1' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))

#define IS_GET(c) ((*c == 'G') && *(c+1) == 'E' && *(c+2) == 'T')
#define IS_HEAD(c) ((*c == 'H') && (*(c+1) == 'E') && *(c+2) == 'A' && *(c+3) == 'D')
#define IS_POST(c) ((*c == 'P') && (*(c+1) == 'O') && *(c+2) == 'S' && *(c+3) == 'T')

#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#define CONTENT_LENGTH "content-length"
#define TRANSFER_ENCODING "transfer-encoding"
#define UPGRADE "upgrade"
#define CHUNKED "chunked"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"

enum {
	rhs_start = 0,
	rhs_method,
	rhs_spaces_before_uri,
	rhs_after_broke_uri,
	rhs_http,
	rhs_http_H,
	rhs_http_HT,
	rhs_http_HTT,
	rhs_http_HTTP,
	rhs_first_major_digit,
	rhs_major_digit,
	rhs_first_minor_digit,
	rhs_minor_digit,
	rhs_spaces_after_digit,
	rhs_almost_done
}request_header_state;

enum {
	rbs_start = 0,
	rbs_key,
	rbs_spaces_before_colon,
	rbs_spaces_after_colon,
	rbs_value,
	rbs_cr,
	rbs_crlf,
	rbs_crlfcr
}request_body_state;

HTTP_PARSE_RESULT http_parse_request_header(http_request_t *rq) {
	/*状态机处理*/
	u_char ch;
	request_header_state = rq->state;
	uint i;
	for(i = rq->cur_pos; i < rq->last; i++) {
		u_char *p = (u_char*)&rq->buf[i % MAX_BUF];
		ch = *p;
		switch(request_header_state) {
			case rhs_start:
				rq->request_start = p;
				if(ch == CR || ch == LF){
					break;
				}
				if(!IS_UPPERCASE(ch)) {
					return HTTP_PARSE_INVALID_METHOD;
				}
				request_header_state = rhs_method;
				break;
			case rhs_method:
				if(ch == ' ') {
					rq->method_end = p;
					u_char* tmp = rq->request_start;
					switch(p - tmp) {
						case 3:
							if(IS_GET(tmp)) {
								rq->method = HTTP_METHOD_GET;
								break;
							}
							break;
						case 4:
							if(IS_HEAD(tmp)) {
								rq->method = HTTP_METHOD_HEAD;
								break;
							}
							if(IS_POST(tmp)) {
								rq->method = HTTP_METHOD_POST;
								break;
							}
							rq->method = HTTP_METHOD_UNKNOW;
							break;
						default:
							rq->method = HTTP_METHOD_UNKNOW;
							break;
					}
					request_header_state = rhs_spaces_before_uri;
					break;
				}
				if(!IS_UPPERCASE(ch)) {
					return HTTP_PARSE_INVALID_METHOD;
				}
				break;
			case rhs_spaces_before_uri:
				if(ch == '/') {
					rq->uri_start = p;
					request_header_state = rhs_after_broke_uri;
					break;
				}
				switch(ch) {
					case ' ':
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_after_broke_uri:
				switch(ch) {
					case ' ':
						rq->uri_end = p;
						request_header_state = rhs_http;
						break;
					default:
						break;
				}
				break;
			case rhs_http:
				switch(ch) {
					case ' ':
						break;
					case 'H':
						request_header_state = rhs_http_H;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_http_H:
				switch(ch) {
					case 'T':
						request_header_state = rhs_http_HT;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_http_HT:
				switch(ch) {
					case 'T':
						request_header_state = rhs_http_HTT;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_http_HTT:
				switch(ch) {
					case 'P':
						request_header_state = rhs_http_HTTP;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_http_HTTP:
				switch(ch) {
					case '/':
						request_header_state = rhs_first_major_digit;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_first_major_digit:
				if(!IS_NUM_GT_ZERO(ch)) {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				rq->major = ch - '0';
				request_header_state = rhs_major_digit;
				break;
			case rhs_major_digit:
				if(ch == '.') {
					request_header_state = rhs_first_minor_digit;
					break;
				}
				if(!IS_NUM(ch)) {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				rq->major = rq->major*10 + ch - '0';
				break;
			case rhs_first_minor_digit:
				if(!IS_NUM(ch)) {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				rq->minor = ch - '0';
				request_header_state = rhs_minor_digit;
				break;
			case rhs_minor_digit:
				if(ch == CR) {
					request_header_state = rhs_almost_done;
					break;
				}
				if(ch == LF) {
					rq->cur_pos = i + 1;
					if(rq->request_end == NULL) {
						rq->request_end = p;
					}
					rq->state = rhs_start;
					return HTTP_PARSE_OK;
				}
				if(ch == ' '){
					request_header_state = rhs_spaces_after_digit;
					break;
				}
				if(!IS_NUM(ch)) {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				rq->minor = rq->minor*10 + ch - '0';
				break;
			case rhs_spaces_after_digit:
				if(ch == ' '){
					break;
				}else if(ch == CR) {
					request_header_state = rhs_almost_done;
					break;
				}else if(ch == LF) {
					rq->cur_pos = i + 1;
					if(rq->request_end == NULL) {
						rq->request_end = p;
					}
					rq->state = rhs_start;
					return HTTP_PARSE_OK;
				}else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case rhs_almost_done:
				rq->request_end = p - 1;
				if(ch == LF) {
					rq->cur_pos = i + 1;
					if(rq->request_end == NULL) {
						rq->request_end = p;
					}
					rq->state = rhs_start;
					return HTTP_PARSE_OK;
				}else{
					return HTTP_PARSE_INVALID_REQUEST;
				}
		}
	}
	rq->cur_pos = i;
	rq->state = request_header_state;
	return HTTP_CONTINUE_PARSE;
}

HTTP_PARSE_RESULT http_parse_request_body(http_request_t *rq) {
	request_body_state = rq->state;
	u_char ch,*p;
	uint i;
	http_header_t *header;
	void* cur_header_key_start;
	void* cur_header_key_end;
	void* cur_header_value_start;
	void* cur_header_value_end;
	for(i = rq->cur_pos; i < rq->last; i++) {
		p = (u_char*)&rq->buf[i % MAX_BUF];
		ch = *p;
		switch(request_body_state) {
			case rbs_start:
				if(ch == CR || ch == LF) {
					break;
				}
				cur_header_key_start = p;
				request_body_state = rbs_key;
				break;
			case rbs_key:
				if(ch == ' '){
					request_body_state = rbs_spaces_before_colon;
					cur_header_key_end = p;
					break;
				}
				if(ch == ':'){
					request_body_state = rbs_spaces_after_colon;
					cur_header_key_end = p;
					break;
				}
				break;
			case rbs_spaces_before_colon:
				if(ch == ' '){
					break;
				}
				if(ch == ':'){
					request_body_state = rbs_spaces_after_colon;
					break;
				}else {
					return HTTP_PARSE_INVALID_HEADER;
				}
				break;
			case rbs_spaces_after_colon:
				if(ch == ' '){
					break;
				}
				request_body_state = rbs_value;
				cur_header_value_start = p;
				break;
			case rbs_value:
				if(ch == CR) {
					request_body_state = rbs_cr;
					cur_header_value_end = p;
				}
				if(ch == LF) {
					request_body_state = rbs_crlf;
					cur_header_value_end = p;
				}
				break;
			case rbs_cr:
				if(ch == LF) {
					request_body_state = rbs_crlf;
					header = (http_header_t*)s_malloc(sizeof(http_header_t));
					header->key_start = cur_header_key_start;
					header->key_end = cur_header_key_end;
					header->value_start = cur_header_value_start;
					header->value_end = cur_header_value_end;
					LIST_ADD_NODE(&(header->list),&(rq->list));
					break;
				}else{
					return HTTP_PARSE_INVALID_HEADER;
				}
				break;
			case rbs_crlf:
				if(ch == CR) {
					request_body_state = rbs_crlfcr;
				}else{
					request_body_state = rbs_key;
					cur_header_key_start = p;
					break;
				}
				break;
			case rbs_crlfcr:
				if(ch == LF) {
					rq->cur_pos = i + 1;
					rq->state = rbs_start;
					return HTTP_PARSE_OK;
				}else{
					return HTTP_PARSE_INVALID_HEADER;
				}
				break;
		}
	}
	rq->cur_pos = i;
	rq->state = request_body_state;
	return HTTP_CONTINUE_PARSE;
}