#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/timer.h>
//systemcall 335
asmlinkage long sys_my_time(void){
	static const long nano = 1000000000;
	struct timespec t;
	getnstimeofday(&t);
	return t.tv_sec * nano + t.tv_nsec;
}
