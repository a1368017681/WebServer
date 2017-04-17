#include "memory_pool.h"
#include <stdlib.h>

void* s_malloc(int size) {
	return malloc(size);
}

void s_free(void *ptr) {
	free(ptr);
}
