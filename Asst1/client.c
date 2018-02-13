#include "my_pthread_t.h"
#include <unistd.h>
my_pthread_mutex_t mutex;
int indicator =0;

int func(int val){
	 my_pthread_mutex_lock(&mutex);
	 indicator ++;
	 printf("%d\n",indicator);
	 my_pthread_mutex_unlock(&mutex);
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));    
	//my_pthread_exit(NULL);
	return  0;

}


int main(){
		
		my_pthread_t t;
		my_pthread_t t1;
		my_pthread_mutex_init(&mutex, NULL);
		my_pthread_create(&t,NULL,(void*)func,(void *)1);
		my_pthread_create(&t1,NULL,(void*)func,(void*)2);
        int i=0;
		while (true){
			}
		return 0;
}

