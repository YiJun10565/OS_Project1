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
void check_policy(char *S);

int main(){
	char S[8];
	scanf("%s", S);
	check_policy(S);

	unsigned int N;
	scanf("%u", &N);

	struct Task *tasks = (struct Task*)malloc(sizeof(struct Task)*N);

	for(int i = 0; i < N; i++){
		scanf("%s", tasks[i].name);
		scanf("%u", &tasks[i].ready_time);
		scanf("%u", &tasks[i].exec_time);
		tasks[i].id = i;
		tasks[i].isInQueue = 0;
		tasks[i].pid = 0;
	}
	if( !strcmp(S, "FIFO") || !strcmp(S, "RR") )
		qsort(tasks, N, sizeof(struct Task), task_cmp_FIFO);
	else 
		qsort(tasks, N, sizeof(struct Task), task_cmp_SJF);

	for(int i = 0; i < N; i++)
		print_task(tasks[i]);
	
	// set # of cpu core
	cpu_set_t mask0, mask1;
	CPU_ZERO(&mask0);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask0);
	CPU_SET(1, &mask1);
	// set main process to cpu 1
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1) == -1){
		fprintf(stderr, "set affinity error\n");
		return -1;
	}

	struct sched_param param_High, param_Low;
	// init param 
	param_Low.sched_priority  = sched_get_priority_max(SCHED_FIFO);
	param_High.sched_priority = param_Low.sched_priority - 1;
	
	// set main process priority to mid
	if( sched_setscheduler(0, SCHED_FIFO, &param_High) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		return -1;
	}
	
	pid_t main_pid = getpid();
	unsigned int *clock = (unsigned int*)malloc(sizeof(unsigned int));
	int *done_task = (int*)malloc(sizeof(int));
	*done_task = 0;
	int *ready_task = (int*)malloc(sizeof(int));
	*ready_task = 0;
	int *next_process = (int*)malloc(sizeof(int));
	*next_process = 0;

	while(*ready_task < N){
		// no process is doing,so main process wait until next process enter
		while(1){
			if(*ready_task < N && *clock != tasks[*ready_task].ready_time)
				break;
			wait_a_unit();
			(*clock)++;
		}
		if(*ready_task < N && *clock == tasks[*ready_task].ready_time){
			chat name[40];
			int current_task = *ready_task;
			(*ready_task)++;
			strcpy(name, tasks[current_task].name);
			printf("It is %d now, and I'm going to insert task %d\n", *clock, *ready_task);
			printf("It's name is %s\n". name);
			int exec_t = tasks[current_task].exec_time;
			pid_t next_pid = 0;
			pid_t pid = fork();
			switch(pid){
				case (pid > 0):
					// main:
					// if need to preempt
					if(sched_setparam(pid, &param_Low)==-1){
						fprintf(stderr, "set param error while create %d:%s\n", pid, name);
						exit -1;
					}
					// if no need to create next process 
					if(*ready_task < N && *clock != tasks[*ready_task].ready_time && !strcmp(S, "PSJF")){
						
						pid_t next_pid = 0;
						int current_exec_time = 0;
						for(int i = 0; i < ready_task; i++){
							if(tasks[i].exec_time != 0 && tasks[i].exec_time < current_exec_time){
								next_pid = tasks[i].pid;
								current_exec_time = tasks[i].exec_time;
							}
						}

					}


				break;
				case 0:
					// child:
					pid = getpid();
					tasks[current_task].pid = pid;
					fprintf(stderr, "%s %d\n", name, pid);
					for(int round = 0; round < exec_t; round++, (*clock)++){
						if(*ready_task < N && tasks[*ready_task].ready_time == *clock){
							sched_setparam(main_pid, &param_High);
							sched_setparam(0, &param_Low);
						}
						if(!strcmp("RR", S) && !(round % 500)){
							int next_task = current_task+1;
							int flag = 0;
							for(; next_task < *ready_task && !flag; next_task++)
								if( tasks[next_task].exec_time )
									flag = 1;
							if(!flag)
								for(next_task = 0; next_task < current_task && !flag; next_task++)
									if( tasks[next_task].exec_time )
										flag = 1;
							if( next_task != current_task ){
								if( sched_setparam(tasks[next_task].pid, &param_High) == -1){
									fprintf(stderr, "RR : child %d\n", pid);
									fprintf(stderr, "set scheduler error\n");
									fprintf(stderr, "Message %s\n", strerror(errno));
									return -1;
								}
								if( sched_setparam(0, &param_Low) == -1){
									fprintf(stderr, "RR : child %d\n", pid);
									fprintf(stderr, "set scheduler error\n");
									fprintf(stderr, "Message %s\n", strerror(errno));
									return -1;
								}
							}

						}
						wait_a_unit();
						(*clock)++;
					}
					(*done_task)++;
					return 0;
				break;
				default:
					printf(stderr, "fork error\n");
					return 1;
				break;
			}
		}
		
		done_task++;
	}
	while(wait(NULL));

	return 0;
}

void check_policy(char *S){
	char policies[4][8] = {"FIFO", "RR", "SJF", "PSJF"};
	for(int i = 0; i <= 4; i++){
		if(i == 4){
			// undefined policy
			fprintf(stderr, "Undefined Policy\n");
			exit(1);
		}
		if(!strcmp(S, policies[i]))
			break;
	}

}

void wait_a_unit(){
	for(volatile unsigned long i = 0; i < 1000000UL; i++);
}
