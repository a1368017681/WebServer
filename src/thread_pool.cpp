#include "thread_pool.h"
#include "memory_pool.h"

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
}threadpool_shutdown_t;

static void *threadpool_thread(void* threadpool);
int threadpool_free(threadpool_t* pool);

threadpool_t *threadpool_create(int thread_count, int queue_size) {
	if(thread_count > MAX_THREADS || thread_count <= 0 || queue_size <= 0 || queue_size > MAX_QUEUE) {
		return NULL;
	}
	threadpool_t *pool = s_malloc(sizeof(threadpool_t));
	if(NULL == pool) {
		goto err;
	}
	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = pool->tail = pool->queue_thread_count = 0;
	pool->shutdown = pool->started = 0;
	
	pool->threads = (pthread_t *)s_malloc(sizeof(pthread_t) * thread_count);
	pool->queue = (task_t*)(s_malloc(sizeof(task_t) * queue_size));
	if(!pool->threads || !pool->queue) {
		goto err;
	}
	if((pthread_mutex_init(&(pool->lock),NULL) != 0) || (pthread_cond_init(&(pool->notify),NULL) != 0)) {
		goto err;
	}

	for(int i = 0; i < thread_count; i++) {
		if(pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool) != 0) {
			threadpool_destroy(pool,0);
			return NULL;
		}
		pool->thread_count++;
		pool->started++;
	}
	return pool;
err:
	if(pool) {
		threadpool_free(pool);
	}
	return NULL;
}

static void *threadpool_thread(void* threadpool) {
	threadpool_t *pool = (threadpool_t*)threadpool;
	task_t task;
	for(;;) {
		pthread_mutex_lock(&(pool->lock));
		while((pool->queue_thread_count == 0) && (!pool->shutdown)) {
			pthread_cond_wait(&(pool->notify),&(pool->lock));
		}
		if((pool->shutdown == immediate_shutdown) || 
			((pool->shutdown == graceful_shutdown) 
			&& (pool->queue_thread_count == 0))) {
			break;
		}
		
		task.fun = pool->queue[pool->head].fun;
		task.arg = pool->queue[pool->head].arg;

		pool->head++;
		pool->head = (pool->head == pool->queue_size)? 0 : pool->head;
		pool->queue_thread_count--;

		pthread_mutex_unlock(&(pool->lock));
		(*(task.fun))(task.arg);
	}
	pool->started--;
	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);
	return NULL;
}

int threadpool_add(threadpool_t *pool, void (*fun)(void *),void *arg) {
	if(pool == NULL || fun == NULL) {
		return threadpool_invalid;
	}
	if(pthread_mutex_lock(&(pool->lock)) != 0) {
		return threadpool_lock_failure;
	}
	int next = pool->tail + 1;
	next = (next == pool->queue_size) ? 0 : next;
	int error = 0;
	do {
		if(pool->queue_thread_count == pool->queue_size){
			error = threadpool_queue_full;
			break;
		}
		if(pool->shutdown) {
			error = threadpool_shutdown;
			break;
		}
		pool->queue[pool->tail].fun = fun;
		pool->queue[pool->tail].arg = arg;

		pool->tail = next;
		pool->queue_thread_count++;

		if(pthread_cond_signal(&(pool->notify)) != 0) {
			error = threadpool_lock_failure;
			break;
		}
	}while(0);
	if(pthread_mutex_unlock(&(pool->lock)) != 0) {
		error = threadpool_lock_failure;
	}
	return error;
}

int threadpool_destroy(threadpool_t *pool, int flags) {
	if(pool == NULL) {
		return threadpool_invalid;
	}
	if(pthread_mutex_lock(&(pool->lock)) != 0) {
		return threadpool_lock_failure;
	}
	int error = 0;
	do {
		if(pool->shutdown) {
			error = threadpool_shutdown;
			break;
		}
		pool->shutdown = (flags & threadpool_graceful) ? graceful_shutdown : immediate_shutdown;
		if((pthread_cond_broadcast(&(pool->notify))) != 0 ||
			(pthread_mutex_unlock(&(pool->lock)) != 0)){
			error = threadpool_lock_failure;
			break;
		}
		for(int i = 0; i < pool->thread_count; i++){
			if(pthread_join(pool->threads[i],NULL) != 0){
				error = threadpool_thread_failure;
			}
		}
	}while(0);
	if(!error){
		threadpool_free(pool);
	}
	return error;
}

int threadpool_free(threadpool_t* pool) {
	if(pool == NULL || pool->started > 0) {
		return -1;
	}
	if(pool->threads) {
		s_free(pool->threads);
		s_free(pool->queue);
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
	}
	s_free(pool);
	return 0;
}
