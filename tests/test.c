#include <priority_queue.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define maxn 100

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

static int cmp(const void* lhs,const void* rhs) {
	return (*(int*)lhs < *(int*)rhs) ? 0 : 1;
}

static int comp(void* lhs,void* rhs) {
	size_t ll = (size_t)lhs;
	size_t rr = (size_t)rhs;
	return (ll < rr) ? 1 : 0;
}

int pq_test_data[maxn];
int pq_test_result[maxn];
static void pq_test() {
	int test_count_before = test_count;
	int test_pass_before = test_pass;
	srand((unsigned)time(NULL));
	for(int i = 0; i < maxn; i++) {
		pq_test_data[i] = rand()%maxn;
		if(!pq_test_data[i]) pq_test_data[i]++;
	}
	for(int i = 0; i < maxn; i++) {
		pq_test_result[i] = pq_test_data[i];
	}
	qsort(pq_test_result,maxn,sizeof(int),cmp);
	
	priority_queue_t pq;
	PQ_STATUS ret = init_pq(&pq,comp,QUEUE_SIZE);
	EXPECT_EQ_INT(INIT_PQ_OK,ret);

	ret = is_empty_pq(&pq);
	EXPECT_EQ_INT(1,ret);

	int sz;
	sz = size_of_pq(&pq);
	EXPECT_EQ_INT(0,sz);

	void* min;
	min = min_of_pq(&pq);
	EXPECT_EQ_INT(NULL,min);

	ret = del_min_pq(&pq);
	EXPECT_EQ_INT(PQ_EMPTY,ret);

	for(int i = 0; i < maxn; i++) {
		ret = insert_item_pq(&pq,(void*)pq_test_data[i]);
		EXPECT_EQ_INT(INSERT_PQ_OK,ret);
		EXPECT_EQ_INT(size_of_pq(&pq),i+1);
	}
	int i = 0;
	while(!is_empty_pq(&pq)) {
		min = min_of_pq(&pq);
		CHECK_EXIT(min != NULL,"get pq_min error%s","");
		EXPECT_EQ_INT(pq_test_result[i],(int)min);
		i++;

		ret = del_min_pq(&pq);
		EXPECT_EQ_INT(DEL_MIN_PQ_OK,ret);
	}
	int count = test_count - test_count_before;
	int pass = test_pass - test_pass_before;
	printf("pq_test : %d/%d (%3.2f%%) passed\n", pass, count, pass * 100.0 / count);
}

static void test() {
	pq_test();
}

int main(){
    test();
    printf("full_test : %d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
