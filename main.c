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
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/stat.h>

// cpu_set_t mask0, mask1;
struct sched_param param_High, param_Low;
struct Task *tasks;
unsigned int *clock;
int *done_task; 
int *ready_task ;
int *current_process;

void Scan_task(int N);
void Init_CPU_and_Param_and_args(int N);


int find_next_process(char *S);

// void set_and_check_affinity(pid_t pid, cpu_set_t *mask);
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
	//set_and_check_affinity(0, &mask1);
	
	// set main process with higher priority
	set_and_check_scheduler_with_lower_priority(0);
	higher_priority(0);
	
	pid_t main_pid = getpid();
	while(*done_task < N){
		// no process is doing loop,so main process wait until next process enter
		//printf("*********Start*************\n");
		//printf("ready_task:%d, done_task:%d, clock=%u.\n", *ready_task, *done_task, *clock);
		//printf("next process: %d:%d:%d\n", *ready_task, tasks[*ready_task].ready_time, tasks[*ready_task].exec_time);
		while(*done_task <= *ready_task){
			if( *done_task == N)
				break;
			if( *ready_task < N && *clock == tasks[*ready_task].ready_time)
				break;
			//printf("Stuck in the While?\n");
			//printf("%d %d\n", *clock, tasks[*ready_task].ready_time);
			if(*done_task < *ready_task){
				int next = find_next_process(S);
				//printf("next = %d, pid = %d, and main pid is %d\n", next, tasks[next].pid, main_pid);
				if(next == -1){
					fprintf(stderr, "find next process error\n");
					fprintf(stderr, "%d < %d", *done_task, *ready_task);
					exit(1);
				}
				higher_priority(tasks[next].pid);
				lower_priority(main_pid);
			}
			else
				wait_a_unit_and_add_clock();
		}
		//a new process enter
		while(*ready_task < N && *clock == tasks[*ready_task].ready_time){
			int current_task = *ready_task;
			(*ready_task)++;
			printf("It is %d now, and I'm going to insert task %d\n", *clock, *ready_task);
			printf("It's name is %s\n", tasks[current_task].name);
			pid_t pid = fork();
			if(pid > 0){
				// main:			
				tasks[current_task].pid = pid;
				// Switch the Child to CPU 0
				//set_and_check_affinity(pid, &mask0);
				// Lower the Child's priority
				lower_priority(pid);
			}
			else if(pid == 0){
				// child:
				// fprintf(stderr, "Hi!!!! I'm %s, and my pid is %d\n", tasks[current_task].name, tasks[current_task].pid);
				
				// **************************
				// TODO
				//struct timespec start_time, end_time;
				//getnstimeofday(&start_time);
				// **************************
				printf("%s %d\n", tasks[current_task].name, tasks[current_task].pid);
				for(int round = 0; tasks[current_task].exec_time > 0; round++, tasks[current_task].exec_time --){
					*current_process = current_task;
					// printf("I'm executing %s at %d, remain time is %d\n", tasks[current_task].name, *clock, tasks[current_task].exec_time);
					if(*ready_task < N && tasks[*ready_task].ready_time == *clock){
						//printf("It's %d time, I'm %s, but a new task %d is coming\n", *clock, tasks[current_task].name, *ready_task );
						higher_priority(main_pid);
						lower_priority(0);
					//	printf("***********Switch???*******\n");
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
							// RR switch
							higher_priority(tasks[next_task].pid);
							lower_priority(0);
						}
						// else: no need to switch, so move on
					}
					wait_a_unit_and_add_clock();
				}
				// this process has done -> back to main
				// ************************
				// TODO
				// getnstimeofday( &end_time);
				//
				// syscall(333, start_time, end_time);
				// printf("%d %ld.%9ld %ld.%9ld", tasks[current_task].pid, start_time.tv_sec, start_time.tv_nsec, end_time.tv_sec, end_time.tv_nsec);
				// *************************
				printf("We have done Task %d at %d\n", current_task, *clock);
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
	//printf("It is %d now, and I'm executing %d\n", *clock, *current_process);
}

void higher_priority(pid_t pid){
	if( sched_setparam(pid, &param_High) == -1){
		fprintf(stderr, "RR : child %d\n", pid);
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		exit(1);
	}
//	printf("Set %d to High priority\n", pid);

}

void lower_priority(pid_t pid){
	if( sched_setparam(pid, &param_Low) == -1){
		fprintf(stderr, "RR : child %d\n", pid);
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		exit(1);
	}
//	printf("Set %d to Low priority\n", pid);
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

void Init_CPU_and_Param_and_args(int N){
	// set # of cpu core
	/*
	CPU_ZERO(&mask0);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask0);
	CPU_SET(1, &mask1);
	*/
	// init param 
	param_High.sched_priority  = sched_get_priority_max(SCHED_FIFO);
	param_Low.sched_priority = param_High.sched_priority - 1;
	// init global arg
	int clock_segmentID = shmget(IPC_PRIVATE, sizeof(unsigned int), S_IRUSR| S_IWUSR );
	clock = (unsigned int*)shmat(clock_segmentID, NULL, 0);
	*clock = 0;
	int done_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	done_task = (int*)shmat(done_segmentID, NULL, 0);
	*done_task = 0;
	int current_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	current_process = (int*)shmat(current_segmentID, NULL, 0);
	*current_process = 0;
	int ready_segmentID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR| S_IWUSR );
	ready_task = (int*)shmat(ready_segmentID, NULL, 0);
	*ready_task = 0;
	int Task_segmentID = shmget(IPC_PRIVATE, sizeof(struct Task) * N , S_IRUSR| S_IWUSR );
	tasks = (struct Task*)shmat(Task_segmentID, NULL, 0);
	*ready_task = 0;
	
}

/*
void set_and_check_affinity(pid_t pid, cpu_set_t *mask){
	if(sched_setaffinity(pid, sizeof(cpu_set_t), mask) == -1){
		fprintf(stderr, "set affinity error\n");
		exit(1);
	}
}
*/
int find_next_process(char *S){
	if( *current_process != -1 && tasks[*current_process].exec_time > 0 && strcmp(S, "PSJF")) return *current_process;
	else if(!strcmp(S, "SJF") || !strcmp(S, "PSJF")){
		int max_exec = 0;
		int next_index = -1;
		for(int i = 0; i < *ready_task; i++)
			if(max_exec == 0 || tasks[i].exec_time < max_exec){
				max_exec = tasks[i].exec_time;
				next_index = i;
			}
		return (max_exec == 0)? -1:next_index;
	}
	else{
		for(int i = 0; i < *ready_task; i++)
			if(tasks[i].exec_time > 0)
				return i;
	}
	return -1;
}
