#include <debug.h>
#include "pq_test.h"
#include "list_test.h"
#include "threadpool_test.h"


static void test() {
	pq_test();
	list_test();
	threadpool_test();
}

int main(){
    test();
    printf("full_test : %d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
