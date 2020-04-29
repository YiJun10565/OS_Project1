#include"Queue.h"
int Queue_is_empty(struct Queue *q){
	return (q->cur_size == 0)? 1 : 0;
}

int Queue_is_full(struct Queue *q){
	return (q->cur_size == q->max_size)? 1 : 0;
}

int EnQueue(struct Queue *q, int index){
	if(Queue_is_full(q)) return -1;
	q->data[q->end] = index;
	q->end = (q->end == (q->max_size)-1)? 0 : (q->end)+1;
	q->cur_size += 1;
	return 0;
}

int DeQueue(struct Queue *q){
	if(Queue_is_empty(q)) return -1;
	int tmp = q->data[q->front];
	q->front = (q->front == (q->max_size)-1)? 0 : (q->front)+1;
	q->cur_size -= 1;
	return tmp;
}

void print_Queue(struct Queue *q){
	fprintf(stderr, "print Queue: ");
	for(int i = q->front; i != q->end; i++, i%=q->max_size){
		fprintf(stderr, "%d ", q->data[i]);
	}
	fprintf(stderr,"\n");
}
