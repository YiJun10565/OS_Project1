#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sys/syscall.h>
//#include<linux/timer.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<sched.h>
#include"task.h"
#include"Queue.h"
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/stat.h>

cpu_set_t mask0, mask1;
struct sched_param param_High, param_Low;
struct sched_param param_test_high, param_test_low;
struct Task *tasks;
unsigned int *clock;
int *done_task; 
int *ready_task ;
int *current_process;

struct Queue *q;

void Scan_task(int N);
void Init_CPU_and_Param_and_args(int N);


int find_next_process(char *S);

void set_and_check_affinity(pid_t pid, cpu_set_t *mask);
void set_and_check_scheduler_with_lower_priority(pid_t pid);

void wait_a_unit_and_add_clock();
void check_policy(char *S);

void higher_priority(pid_t pid);
void lower_priority(pid_t pid);

int main(){
	char S[8];
	scanf("%s", S);
	check_policy(S);

	unsigned int N;
	scanf("%u", &N);
	Init_CPU_and_Param_and_args(N);
	Scan_task(N);

	if( !strcmp(S, "FIFO") || !strcmp(S, "RR") )
		qsort(tasks, N, sizeof(struct Task), task_cmp_FIFO);
	else 
		qsort(tasks, N, sizeof(struct Task), task_cmp_SJF);

	//for(int i = 0; i < N; i++)
	//	print_task(tasks[i]);
	
	// set main process to cpu 1
	set_and_check_affinity(0, &mask1);
	
	// set main process with higher priority
	set_and_check_scheduler_with_lower_priority(0);
	higher_priority(0);
	
	pid_t main_pid = getpid();
	while(*done_task < N){
		// no process is doing loop,so main process wait until next process enter
		while(*done_task <= *ready_task){
			if( *done_task == N)
				break;
			if( *ready_task < N && *clock == tasks[*ready_task].ready_time)
				break;
			//printf("Stuck in the While?\n");
			if(*done_task < *ready_task){
				//printf("%d %d\n", *clock, tasks[*ready_task].ready_time);
				int next = find_next_process(S);
				//fprintf(stderr, "Switch to %s\n", tasks[next].name);
				//for(int i = 0; i < N; i++)
				//	print_task(tasks[i]);
				higher_priority(tasks[next].pid);
				lower_priority(0);
			}
			else
				wait_a_unit_and_add_clock();
		}
		//a new process enter
		while(*ready_task < N && *clock == tasks[*ready_task].ready_time){
			int current_task = *ready_task;
			(*ready_task)++;
			//fprintf(stderr, "It is %d now, and I'm going to insert task %d\n", *clock, *ready_task);
			//fprintf(stderr, "It's name is %s\n", tasks[current_task].name);
			pid_t pid = fork();
			if(pid > 0){
				// main:			
				tasks[current_task].pid = pid;
				//set_and_check_affinity(pid, mask0);
				// Lower the Child's priority
				// push Queue:
				EnQueue(q, current_task);
				//print_Queue(q);
				lower_priority(pid);
			}
			else if(pid == 0){
				// child:
				
				long start_time = syscall(335);
				fprintf(stderr, "Starts %s:%d at %u\n", tasks[current_task].name, tasks[current_task].pid, *clock);
				printf("%s %d\n", tasks[current_task].name,  tasks[current_task].pid);
				for(int round = 0; tasks[current_task].exec_time > 0; round++, tasks[current_task].exec_time --){
					*current_process = current_task;
					// insert new chlid
					if(*ready_task < N && tasks[*ready_task].ready_time == *clock){
						//fprintf(stderr, "New Child!!\n");
						higher_priority(main_pid);
						lower_priority(0);
					}
					
					if(!strcmp("RR", S) && round > 0 && !(round % 500) && q->cur_size > 1){
						// front -> end
						EnQueue(q, q->data[q->front]);
						DeQueue(q);
						//print_Queue(q);
						higher_priority(tasks[q->data[q->front]].pid);
						lower_priority(0);
					}
					wait_a_unit_and_add_clock();
				}
				long end_time = syscall(335);
				syscall(334, tasks[current_task].pid, start_time, end_time);
			 
				fprintf(stderr, "%s is done at %d\n", tasks[current_task].name, *clock);
				// Queue pop
				DeQueue(q);
				//print_Queue(q);
				(*done_task)++;
				higher_priority(main_pid);
				exit(0);
			}
			else{
				fprintf(stderr, "fork error\n");
				return 1;
			}	
		}
	}
	while(wait(NULL)>0);

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

void wait_a_unit_and_add_clock(){
	for(volatile unsigned long i = 0; i < 1000000UL; i++);
	(*clock)++;
	if(!(*clock)%1000) printf("clock = %d\n", *clock);
}

void higher_priority(pid_t pid){
	if( sched_setparam(pid, &param_High) == -1){
		fprintf(stderr, "RR : child %d\n", pid);
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		exit(1);
	}
	//fprintf(stderr, "Set %d to High prioirty\n", pid);
}

void lower_priority(pid_t pid){
	if( sched_setparam(pid, &param_Low) == -1){
		fprintf(stderr, "RR : child %d\n", pid);
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		exit(1);
	}
	//fprintf(stderr, "Set %d to low prioirty\n", pid);
}

void set_and_check_scheduler_with_lower_priority(pid_t pid){
	if( sched_setscheduler(pid, SCHED_FIFO, &param_Low) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		exit(1);
	}
}

void Scan_task(int N){
	for(int i = 0; i < N; i++){
		scanf("%s", tasks[i].name);
		scanf("%u", &tasks[i].ready_time);
		scanf("%u", &tasks[i].exec_time);
		tasks[i].id = i;
		tasks[i].pid = 0;
	}

}
void set_and_check_affinity(pid_t pid, cpu_set_t *mask){
	if(sched_setaffinity(pid, sizeof(cpu_set_t), mask) == -1){
		fprintf(stderr, "set pid=%d affinity error\n", pid);
		exit(1);
	}
}

void Init_CPU_and_Param_and_args(int N){
	// init cpu
	CPU_ZERO(&mask0);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask0);
	CPU_SET(1, &mask1);

	
	// init param 
	param_High.sched_priority  = sched_get_priority_max(SCHED_FIFO);
	param_Low.sched_priority = param_High.sched_priority - 50;
	// init global arg
	// Clock
	int clock_segmentID = shmget(IPC_PRIVATE, sizeof(unsigned int), S_IRUSR| S_IWUSR );
	clock = (unsigned int*)shmat(clock_segmentID, NULL, 0);
	*clock = 0;
	// Done
	int done_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	done_task = (int*)shmat(done_segmentID, NULL, 0);
	*done_task = 0;
	int queue_segmentID = shmget(IPC_PRIVATE, sizeof(struct Queue), S_IRUSR | S_IWUSR);
	q = (struct Queue *)shmat(queue_segmentID, NULL, 0);
	int queue_data_segmentID = shmget(IPC_PRIVATE, sizeof(int) * (N+5), S_IRUSR | S_IWUSR);
	q->data = (int *)shmat(queue_data_segmentID, NULL, 0);
	q->front = q->end = q->cur_size = 0;
	q->max_size = N+5;

	int current_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	current_process = (int*)shmat(current_segmentID, NULL, 0);
	*current_process = 0;
	
	// Ready_task
	int ready_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	ready_task = (int*)shmat(ready_segmentID, NULL, 0);
	*ready_task = 0;
	
	int Task_segmentID = shmget(IPC_PRIVATE, sizeof(struct Task) * N , S_IRUSR| S_IWUSR );
	tasks = (struct Task*)shmat(Task_segmentID, NULL, 0);
	
}

int find_next_process(char *S){
	if(!strcmp(S, "FIFO") || !strcmp(S, "RR"))
		return q->data[q->front];
	if( tasks[*current_process].exec_time > 0 && strcmp(S, "PSJF")) return *current_process;
	//fprintf(stderr, "Find SJF in %d tasks\n", *ready_task);
	int min_exec = tasks[0].exec_time;
	int next_index = 0;
	for(int i = 1; i < *ready_task; i++)
		if( tasks[i].exec_time != 0 && (min_exec == 0|| tasks[i].exec_time < min_exec)){
			min_exec = tasks[i].exec_time;
			next_index = i;
		}
	return next_index;
}
