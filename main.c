#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<errno.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>
#include"task.h"


void wait_a_unit();

int main(){
	char S[8];
	unsigned int N;
	scanf("%s", S);
	scanf("%d", &N);
	struct Task *tasks = (struct Task*)malloc(sizeof(struct Task)*N);
	struct Task *exec_queue = (struct Task*)malloc(sizeof(struct Task)*N);
	unsigned int exec_queue_front = 0;
	unsigned int exec_queue_end = 0;

	for(int i = 0; i < N; i++){
		scanf("%s", tasks[i].name);
		scanf("%u", &tasks[i].ready_time);
		scanf("%u", &tasks[i].exec_time);
		tasks[i].isInQueue = 0;
		tasks[i].pid = 0;
	}
	qsort(tasks, N, sizeof(struct Task), task_cmp);
	for(int i = 0; i < N; i++)
		print_task(tasks[i]);
	
	// set # of cpu core
	/*
	cpu_set_t mask, mask1;
	CPU_ZERO(&mask);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask);
	CPU_SET(1, &mask1);
	
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1) == -1){
		fprintf(stderr, "set affinity error\n");
		return -1;
	}*/

	struct sched_param param1, param2, param3;// param1 has larger priority
	param1.sched_priority = sched_get_priority_max(SCHED_FIFO);
	param2.sched_priority = para1.sched_priority + 1;
	
	if( sched_setscheduler(0, SCHED_FIFO, &param1) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		return -1;
	}
	/*
	struct timeval start, end;
	struct timespec start_n, end_n;

	unsigned int *clock = (unsigned int *)malloc(sizeof(unsigned int));
	int done_task = 0;
	int ready_task = 0;
	pid_t *pid_table = (pid_t*)malloc(sizeof(pid_t) * N);*/
/*	while(done_task < N){
		// no process is doing,so main process wait until next process enter
		while(1){
			if(ready_task < N && *clock != tasks[ready_task].ready_time){
				break;
			}
			if(exec_queue_front < exec_queue_end)
				break;
			wait_a_unit((*clock)++);
		}
		if(ready_task < N && *clock == tasks[ready_task].ready_time){
			pid_t pid = fork();
			switch(pid){
				case (pid > 0):
					
				break;
				case 0:
					// main:
					// if need to preempt
					
				break;
				default:
					printf(stderr, "fork error\n");
					return 1;
				break;
			}
		}
		
		done_task++;
	}
	//while(exec_arr_size > 0)	exec_arr_size--;
*/
	return 0;
}


void wait_a_unit(){
	for(volatile unsigned long i = 0; i < 1000000UL; i++);
}
