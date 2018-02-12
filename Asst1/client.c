#include "my_pthread_t.h"
#include <unistd.h>
my_pthread_mutex_t mutex;
int indicator =0;

int func(){
	 my_pthread_mutex_lock(&mutex);
	 indicator ++;
	 printf("%d",indicator);
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
    
     while(true){
        continue;
     }
	//my_pthread_exit(NULL);
	return  0;

}

int main(){
		
		my_pthread_t t;
		my_pthread_t t1;
		my_pthread_mutex_init(&mutex, NULL);
		my_pthread_create(&t,NULL,(void*)func,(void *)5);
		my_pthread_create(&t1,NULL,(void*)func,NULL);
        int i=0;
		my_pthread_join(t1,NULL);
		return 0;
}

