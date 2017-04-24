#include <debug.h>
#include <thread_pool.h>
#include "test_util.h"

#define NUM 8
pthread_mutex_t lock;
size_t sum = 0;

static void sum_n(void* arg) {
	size_t n = (size_t)arg;
	int ret;
	ret = pthread_mutex_lock(&lock);
	EXPECT_EQ_INT(0,ret);

	sum += n;
	ret = pthread_mutex_unlock(&lock);
	EXPECT_EQ_INT(0,ret);
}

void threadpool_test() {
	int test_count_before = test_count;
	int test_pass_before = test_pass;
	int ret = pthread_mutex_init(&lock,NULL);
	EXPECT_EQ_INT(0,ret);
	threadpool_t *tp = threadpool_create(NUM,65535);
	CHECK_EXIT(tp != NULL,"threadpool_create error!%s","");
	size_t i;
	for(i = 0; i < 100; i++) {
		ret = threadpool_add(tp, sum_n, (void *)i);
		EXPECT_EQ_INT(0,ret);
	}
	CHECK_EXIT(threadpool_destroy(tp,1) == 0,"threadpool_destroy error!%s","");
	EXPECT_EQ_INT(4950,sum);
	/*int count = test_count - test_count_before;
	int pass = test_pass - test_pass_before;
	printf("threadpool_test : %d/%d (%3.2f%%) passed\n", pass, count, pass * 100.0 / count);*/
	printf("threadpool_test : threadpool_test passed\n");	
}