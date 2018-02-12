#include "my_pthread_t.h"
#include <unistd.h>
my_pthread_mutex_t mutex;


int func(int indicator){
	int lockable = my_pthread_mutex_lock(&mutex);
	if (lockable == 0)
			printf("successful initial lock\n");
	else
	{
		indicator = 10;
		printf("failed initial lock\n");
	}
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
    
	my_pthread_exit(NULL);
	return  0;

}

int main(){
		
		my_pthread_t t;
		int indicator = 0;
		my_pthread_mutex_init(&mutex, NULL);
		my_pthread_create(&t,NULL,(void*)func,(void *)5);
		int locked = my_pthread_mutex_lock(&mutex);
		if (locked == -1)
		{
			printf("DEADLOCK WOOOO!\n");
		}
		return 0;
}

