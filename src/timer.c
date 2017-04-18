#include "timer.h"
#include <sys/time.h>
#include "util.h"

priority_queue_t Timer;
uint Current_time; //毫秒为单位

static void update_time();

static int cmp(void* lhs,void* rhs) {
	timer_node_t *timer_lhs = (timer_node_t*)lhs;
	timer_node_t *timer_rhs = (timer_node_t*)rhs;
	if (timer_lhs->time < timer_rhs->time) {
		return 1;
	}
	return 0;
}

/*初始化timer*/
TIMER_STATUS init_timer() {
	int ret = init_pq(&Timer,cmp,QUEUE_SIZE);
	CHECK_EXIT(INIT_PQ_OK == ret,"timer_init init pq error!%s","");
	update_time();
	return INIT_TIMER_OK;
}

static void update_time() {
	struct timeval tv;
	struct timezone tz;
	
	int ret = gettimeofday(&tv,&tz);
	CHECK(ret == 0,"update_time : gettimeofday error!%s","");

	Current_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	DEBUG("update_time : Current_time is %u",Current_time);
}

int find_timer() {
	int time = TIMER_INFINITE;
	while(!is_empty_pq(&Timer)) {
		DEBUG("find_timer in pq!%s","");
		update_time();
		
		timer_node_t* node = (timer_node_t*)min_of_pq(&Timer);
		CHECK(NULL != node,"find_timer error for pq is empty!%s","");

		if(node->closed) { //远端关闭连接
			int ret = del_min_pq(&Timer);
			CHECK(ret == DEL_MIN_PQ_OK,"node closed del_min_pq error!%s","");
			s_free(node);
			continue;
		}

		time = (int)(node->time - Current_time);
		DEBUG("find_timer,node time = %u ,cur = %u",node->time,Current_time);
		time = MAX(0,time);
		break;
	}
	return time;
}

TIMER_STATUS handle_expire_timers() {
	DEBUG("in handle_expire_timers!%s","");
	while(!is_empty_pq(&Timer)) {
		DEBUG("handle_expire_timers! size = %d",size_of_pq(&Timer));
		update_time();
		
		timer_node_t* node = (timer_node_t*)min_of_pq(&Timer);
		CHECK(NULL != node,"handle_expire_timers: min_of_pq error!%s","");

		if(node->closed) { //远端关闭连接
			int ret = del_min_pq(&Timer);
			CHECK(ret == DEL_MIN_PQ_OK,"handle_expire_timers: del_min_pq error!%s","");
			s_free(node);
			continue;
		}

		if(node->time > Current_time) 
			return HANDLE_EXPIRE_TIMERS_OK;
		if(node->timer_handler) {
			node->timer_handler(node->request);
		}

		int ret = del_min_pq(&Timer);
		CHECK(ret == DEL_MIN_PQ_OK,"handle_expire_timers: del_min_pq error!%s","");
		free(node);		
	}
	return HANDLE_EXPIRE_TIMERS_OK;
}

TIMER_STATUS add_timer(http_request_t* rq,uint timeout,timer_handler_ptr handler) {
	timer_node_t* node = (timer_node_t*)s_malloc(sizeof(timer_node_t));
	CHECK(NULL != node,"add_timer: s_malloc error!%s","");
	node->request = rq;
	rq->timer = node;
	update_time();
	node->time = Current_time + timeout;
	DEBUG("in add_timer time = %u",node->time);
	node->timer_handler = handler;
	node->closed = 0;

	int ret = insert_item_pq(&Timer,node);
	CHECK(INSERT_PQ_OK == ret,"add_timer: insert_item_pq error!%s","");
	return ADD_TIMER_OK;
}

TIMER_STATUS del_timer(http_request_t* rq) {
	DEBUG("in del_timer");
	update_time();
	timer_node_t* node = rq->timer;
	CHECK(node != NULL,"del_timer: rq->timer is NULL%s","");
	node->closed = 1;
	return DEL_TIMER_OK;
}