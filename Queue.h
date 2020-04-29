#include<stdio.h>

struct Queue{
	int front, end;
	int max_size, cur_size;
	int *data;
};

int Queue_is_empty(struct Queue *q);
int Queue_is_full(struct Queue *q);
int EnQueue(struct Queue *q, int index);
int DeQueue(struct Queue *q);

void print_Queue(struct Queue *q);
