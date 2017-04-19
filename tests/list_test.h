#include <list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "test_util.h"

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

void display(list_t* head) {
	
}

void list_test() {
	
}