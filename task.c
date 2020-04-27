#include"task.h"

int task_cmp(const void *pa, const void *pb){
	struct Task *a = (struct Task*)pa;
	struct Task *b = (struct Task*)pb;
	if(a->ready_time < b->ready_time)
		return -1;
	else if(a->ready_time > b->ready_time)
		return 1;
	if( a->exec_time < b->exec_time)
		return -1;
	else 
		return 1;
}

void swap_task(struct Task exec_arr[], int id1, int id2){
	char tmp_name[40];
	strcpy(tmp_name, exec_arr[id1].name);
	strcpy(exec_arr[id1].name, exec_arr[id2].name);
	strcpy(exec_arr[id2].name, tmp_name);
	
	int rt = exec_arr[id1].ready_time;
	exec_arr[id1].ready_time = exec_arr[id2].ready_time;
	exec_arr[id2].ready_time = rt;
	
	rt = exec_arr[id1].exec_time;
	exec_arr[id1].exec_time = exec_arr[id2].exec_time;
	exec_arr[id2].exec_time = rt;
	return ;
}

void print_task(struct Task task){
	printf("name = %s\n", task.name);
	printf("ready time = %d\n", task.ready_time);
	printf("executing time = %d\n", task.exec_time);
	if(task.isInQueue){
		printf("It is in the queue, and its pid is %d\n", task.pid);
	}
	else
		printf("It is not in the queue.\n");
	return;
}
