#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include "util.h"
#include "debug.h"

#define QUEUE_SIZE 10

typedef int (*priority_queuq_cmp_ptr)(void* lhs,void* rhs);

typedef struct {
	uint size; /*当前大小*/
	uint capacity; /*pq容量*/
	priority_queuq_cmp_ptr cmp; /*比较函数*/
	void **priority_queue; 
}priority_queue_t;

typedef enum {
	INIT_PQ_OK = 0,
	INIT_PQ_FAIL,
	DEL_MIN_PQ_OK,
	DEL_MIN_PQ_FAIL,
	INSERT_PQ_OK,
	INSERT_PQ_FAIL,
	RESIZE_PQ_OK,
	RESIZE_PQ_FAIL,
	PQ_EMPTY,
	PQ_ERROR_UNKNOWN
}PQ_STATUS;

PQ_STATUS init_pq(priority_queue_t* ptr,priority_queuq_cmp_ptr cmp,uint size);
int is_empty_pq(priority_queue_t* ptr);
uint size_of_pq(priority_queue_t* ptr);
void* min_of_pq(priority_queue_t* ptr);
PQ_STATUS insert_item_pq(priority_queue_t* ptr,void *item);
PQ_STATUS del_min_pq(priority_queue_t* ptr);

#endif
