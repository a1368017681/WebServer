#include "timer.h"
#include <time.h>


priority_queue_t Timer;
int Current_time;

/*初始化timer*/
TIMER_STATUS init_timer() {
	return INIT_TIMER_OK;
}

int find_timer() {
	return 0;
}

TIMER_STATUS handle_expire_timers() {
	return HANDLE_EXPIRE_TIMERS_OK;
}
