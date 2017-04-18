#ifndef _TIMER_H_
#define _TIMER_H_

#include "priority_queue.h"
#include "http_request.h"
#include "memory_pool.h"


#define TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500

typedef int (*timer_handler_ptr)(http_request_t* request); 

typedef enum {
	INIT_TIMER_OK = 0,
	INIT_TIMER_FAIL,
	INIT_TIMER_UNKNOWN,
	ADD_TIMER_OK,
	ADD_TIMER_FAIL,
	DEL_TIMER_OK,
	DEL_TIMER_FAIL,
	NOT_FOUND_TIMER,
	HANDLE_EXPIRE_TIMERS_OK,
	HANDLE_EXPIRE_TIMERS_FAIL
}TIMER_STATUS;

typedef struct {
	uint time;
	int closed;
	timer_handler_ptr timer_handler;
	http_request_t* request;
}timer_node_t;

TIMER_STATUS init_timer();
int find_timer();
TIMER_STATUS handle_expire_timers();
TIMER_STATUS add_timer(http_request_t* rq,uint timeout,timer_handler_ptr handler);
TIMER_STATUS del_timer(http_request_t* rq);

#endif