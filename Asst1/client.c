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
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
	return  0;

}

int main(){
		my_pthread_t t;
		typedef struct args {
			int arg1;
			int arg2;

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
}

