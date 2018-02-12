#include "my_pthread_t.h"
#include <unistd.h>
int func(int indicator, my_pthread_mutex_t * mutex){
	printf("new thread created\n");
	printf("HELLO WORLD\n");
	printf("KEK\n");
	int lockable = my_pthread_mutex_lock(mutex);
	if (lockable == -1)
			printf("WOOOO DEADLOCK BITCHES\n");
	else
	{
		indicator = 10;
	}
    while(i < 2000000000){printf("IM THREAD1\n");}
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
    my_pthread_exit(NULL);
	return  0;

}

int main(){

		}args;
		int indicator = 0;
		args arg;
	   arg.arg1 = 5;
   	    arg . arg2 = 6;
		my_pthread_mutex_t * mutex;
		my_pthread_mutex_init(mutex, NULL);
		int locked = my_pthread_mutex_lock(mutex);
		if (locked == 1)
				printf("locked successfully!\n");
		indicator = 1;
		my_pthread_create(&t,NULL,(void*)func,(void *)5);
	//	printf("Back in main bois");
        testThreads(30);
		//printf("adress before create %x\n",&z);
        //my_pthread_create(&z,NULL,(void*)func1,NULL);
		//printf("adress of thread after creation %x \n",&z);
		//printf("JOINED\n");
		//int *x;
		//my_pthread_join(z,(void **)&x);
		//printf("%d\n",*x);
		//printf("Back in main bois");
        //
        //Main Thread should run super long as to not end the process
        int *x;
        while(true){printf("IN MAIN\n");}
        return 0;
}

