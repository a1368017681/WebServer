#include "priority_queue.h"
#include "memory_pool.h"
#include <string.h>
#include <stdlib.h>

PQ_STATUS init_pq(priority_queue_t* ptr,priority_queuq_cmp_ptr cmp,uint size) {
	ptr->priority_queue = (void **)s_malloc(sizeof(void*)*(size+1));
	if(NULL == ptr->priority_queue) { //分配内存失败
		LOG_ERROR("priority_queue malloc failed!%s","");
		return INIT_PQ_FAIL;
	}

	ptr->size = 0;
	ptr->capacity = size + 1;
	ptr->cmp = cmp;
	return INIT_PQ_OK;
}

int is_empty_pq(priority_queue_t* ptr) {
	if(ptr->size == 0) {
		return 1;
	}
	return 0;
}

uint size_of_pq(priority_queue_t* ptr) {
	return ptr->size;
}

void* min_of_pq(priority_queue_t* ptr) {
	if(is_empty_pq(ptr)) {
		return NULL;
	}
	return ptr->priority_queue[1];
}

static int resize(priority_queue_t* ptr,uint capacity) {
	if(ptr->size >= capacity) {
		LOG_ERROR("resize new capacity too small!%s","");
		return -1;
	}

	void **capacity_ptr = (void**)s_malloc(sizeof(void*) * capacity);
	if(NULL == capacity_ptr) {
		LOG_ERROR("resize malloc error!%s","");
		return -1;
	}

	memmove(capacity_ptr,ptr->priority_queue,sizeof(void *) * (ptr->size+1));
	s_free(ptr->priority_queue);
	ptr->priority_queue = capacity_ptr;
	ptr->capacity = capacity;
	return 0;
}

static void swap(priority_queue_t* ptr,uint lhs,uint rhs) {
	void *temp = ptr->priority_queue[lhs];
	ptr->priority_queue[lhs] = ptr->priority_queue[rhs];
	ptr->priority_queue[rhs] = temp;
}

static void swim(priority_queue_t* ptr,uint index) {
	uint k = index;
	while(k > 1 && ptr->cmp(ptr->priority_queue[k],ptr->priority_queue[k>>1])) {
		swap(ptr,k,k>>1);
		k >>= 1;
	}
}

PQ_STATUS insert_item_pq(priority_queue_t* ptr,void *item) {
	uint size = ptr->size;
	if((size + 1) == ptr->capacity) {
		int ret = resize(ptr,ptr->capacity * 2);
		if(ret) {
			LOG_ERROR("insert_item pq resize error!%s","");
			return RESIZE_PQ_FAIL;
		}
	}

	/*优先队列添加元素，重新维护优先队列结构*/
	ptr->priority_queue[++ptr->size] = item;
	swim(ptr,ptr->size);
	
	return INSERT_PQ_OK;
}

static void sink(priority_queue_t* ptr,uint index) {
	uint k = index;
	while((k<<1) <= ptr->size) {
		uint j = (k << 1);
		if(j < ptr->size && ptr->cmp(ptr->priority_queue[j+1],ptr->priority_queue[j])) {
			j = j + 1;
		}
		if(!ptr->cmp(ptr->priority_queue[j],ptr->priority_queue[k]))
			break;
		swap(ptr,j,k);
		k = j;
	}
}

PQ_STATUS del_min_pq(priority_queue_t* ptr) {
	if(is_empty_pq(ptr)) {
		return PQ_EMPTY;
	}
	/*删除队首元素，重新维护优先队列结构*/
	swap(ptr,1,ptr->size);
	ptr->size--;
	sink(ptr,1);

	if(ptr->size > 0 && ptr->size <= (ptr->capacity - 1)/4 ){
		int ret = resize(ptr,ptr->capacity/2);
		if(ret) {
			LOG_ERROR("del_min pq resize error!%s","");
			return DEL_MIN_PQ_FAIL;
		}
	}
	return DEL_MIN_PQ_OK;
}

