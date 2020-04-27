#include<stdio.h>
#include<unistd.h>
#include<string.h>
struct Task{
	char name[40];				// N
	unsigned int ready_time;	// R
	unsigned int exec_time;		// T
	int isInQueue;
	pid_t pid;
};
int task_cmp(const void *pa, const void *pb);
void swap_task(struct Task exec_arr[], int id1, int ind2);
void print_task(struct Task task);
