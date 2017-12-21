#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
typedef struct _Node Node;
typedef struct _Queue Queue;
struct _Node {
	void *data;
	struct _Node *next;
};

struct _Queue {
	Node* head;
	Node* tail;
};
Queue* queue_init();
void queue_enqueue(Queue *q, gpointer data);
gpointer queue_dequeue(Queue *q);
