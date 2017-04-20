#ifndef _LIST_H_
#define _LIST_H_

#ifndef NULL
#define NULL 0
#endif

typedef struct list_t{
	struct list_t *prev;
	struct list_t *next;
}list_t;

#define LIST_INIT_HEAD(ptr) do { \
	list_t *tmp_ptr = (list_t*)(ptr); \
	(tmp_ptr)->prev = (tmp_ptr); \
	(tmp_ptr)->next = (tmp_ptr); \
}while(0)

#define LIST_ADD_NODE(new_node,head) do { \
	(new_node)->next = ((head)->next); \
	((head)->next)->prev = (new_node); \
	(head)->next = (new_node); \
	(new_node)->prev = (head); \
}while(0)

#define LIST_ADD_NODE_TAIL(new_node,head) do { \
	(new_node)->next = (head); \
	(head)->prev = (new_node); \
	((head)->prev)->next = (new_node); \
	(new_node)->prev = (head)->prev; \
}while(0)

#define LIST_DEL_NODE(entry) do { \
	((entry)->prev)->next = (entry)->next; \
	((entry)->next)->prev = (entry)->prev; \
}while(0)

static int list_empty(list_t* head) {
	return (head->next == head) && (head->prev == head);
}

#define OFFSET(TYPE,MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define CONTAINER_OF(ptr,type,member) ({ \
	const __typeof__( ((type*)0)->member) *__mptr = (ptr); \
	(type*)( (char*) __mptr - OFFSET(type,member)); \
})

#define LIST_ENTRY(ptr,type,member) \
	CONTAINER_OF(ptr,type,member)

#define LIST_FOR_EACH(pos,head) \
	for(pos = (head)->next; pos != head; pos = (pos)->next)

#define LIST_FOR_EACH_PREV(pos,head) \
	for(pos = (head)->prev; pos != head; pos = (pos)->prev) 

#endif
