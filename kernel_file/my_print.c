#include<linux/linkage.h>
#include<linux/kernel.h>
//systemcall 334
asmlinkage void sys_print(int pid, long start_time, long end_time){
	static const long nano = 1000000000;
	printk(KERN_INFO "[Project1] %d %ld.%09ld %ld.%09ld\n", pid, start_time/nano, start_time%nano, end_time/nano, end_time%nano);
}
