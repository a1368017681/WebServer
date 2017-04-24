#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "util.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#define DEFAULT_THREAD_NUM 4
#define MAX_THREADS 16
#define MAX_QUEUE 65536

typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
}threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
}threadpool_destroy_flags_t;

typedef struct task_s{
	void(*fun)(void*);
	void *arg;
}task_t;

typedef struct {
	pthread_t *threads;
	pthread_cond_t notify; 
	pthread_mutex_t lock;
	task_t *queue;
	int queue_size;
	int head;
	int tail;
	int queue_thread_count;
	int thread_count;
	int shutdown;
	int started;
}threadpool_t;

threadpool_t *threadpool_create(int thread_count, int queue_size);

int threadpool_add(threadpool_t *pool, void (*fun)(void *),void *arg);

int threadpool_destroy(threadpool_t *pool, int flags);

#endif