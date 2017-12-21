#include "cqueue.h"
Queue*
queue_init() {
	Queue *q = g_slice_new(Queue);
	q->head = g_slice_new0(Node);
	q->tail = q->head;
	return q;
}

void queue_enqueue(Queue *q, gpointer data) {
	Node *node, *tail, *next;

	node = g_slice_new(Node);
	node->data = data;
	node->next = NULL;

	while (TRUE) {
		tail = q->tail;
		next = tail->next;
		if (tail != q->tail)
			continue;

		if (next != NULL) {
			__sync_bool_compare_and_swap(&q->tail, tail, next);
			continue;
		}

		if (__sync_bool_compare_and_swap(&tail->next, NULL, node))
			break;
	}

	__sync_bool_compare_and_swap(&q->tail, tail, node);
}

gpointer queue_dequeue(Queue *q) {
	Node *node, *head, *tail, *next;
	gpointer data;
	while (TRUE) {
		head = q->head;
		tail = q->tail;
		next = head->next;
		if (head != q->head)
			continue;

		if (next == NULL)
			return NULL; // Empty

		if (head == tail) {
			__sync_bool_compare_and_swap(&q->tail, tail, next);
			continue;
		}

		data = next->data;
		if (__sync_bool_compare_and_swap(&q->head, head, next))
			break;
	}

	//g_slice_free(Node, head); // This isn't safe
	return data;
}
/*
 int main()
 {
 Queue* queue = queue_init();
 int a = 3;
 queue_enqueue(queue,&a);
 printf("%d\n",*(int*)queue_dequeue(queue));
 return 0;
 }*/
