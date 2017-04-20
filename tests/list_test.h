#include <list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "test_util.h"

#define maxn 100
typedef struct {
	void* arg;
	list_t list;
}t_header;

typedef struct {
	void* arg1;
	void* arg2;
	list_t list;
}t_struct;

void add_node(void* arg,list_t* head) {
	t_struct *s = (t_struct*)malloc(sizeof(t_struct));
	s->arg1 = arg;
	s->arg2 = arg;
	LIST_ADD_NODE(&(s->list),head);
}

void del_node(list_t* entry) {
	LIST_DEL_NODE(entry);
	t_struct *s = LIST_ENTRY(entry,t_struct,list);
	free(s);
}


void list_test() {
	int test_count_before = test_count;
	int test_pass_before = test_pass;
	t_header t_h;
	LIST_INIT_HEAD(&t_h.list);

	int ret = list_empty(&t_h.list);
	EXPECT_EQ_INT(1,ret);

	int i;
	for(i = 0; i < maxn; i++) {
		add_node((void*)i,&(t_h.list));
	}
	ret = list_empty(&t_h.list);
	EXPECT_EQ_INT(0,ret);
	list_t* pos;
	t_struct *tmp;
	i = maxn - 1;
	LIST_FOR_EACH(pos,&t_h.list) {
		tmp = LIST_ENTRY(pos,t_struct,list);
		EXPECT_EQ_INT(i,(int)tmp->arg1);
		i--;
	}
	for(i = 0; i < 32; i++) {
		del_node(t_h.list.next);
	}
	i = maxn - 32 - 1;
	LIST_FOR_EACH(pos,&t_h.list) {
		tmp = LIST_ENTRY(pos,t_struct,list);
		EXPECT_EQ_INT(i,(int)tmp->arg1);
		i--;
	}
	int count = test_count - test_count_before;
	int pass = test_pass - test_pass_before;
	printf("list_test : %d/%d (%3.2f%%) passed\n", pass, count, pass * 100.0 / count);
}