#include "my_pthread_t.h"
#include <unistd.h>
#include <pthread.h>
int func(int x){
	printf("new thread created\n");
	printf("HELLO WORLD\n");
	printf("KEK\n");
	printf("x is %d\n",x);
    int i = 0;
    while(i < 2000000000){printf("IM THREAD1\n");}
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
    my_pthread_exit(NULL);
	return  0;

}
int func1(){
    printf("Currently in second thread\n");
//my_pthread_yield();
	int *x = malloc(sizeof(int));
	*x = 9;
	printf("prepare to exit on the right \n");
	pthread_exit(x);
    return 0;
}


int main(){
        pthread_t z;
		printf("adress before create %x\n",&z);
        pthread_create(&z,NULL,(void*)func1,NULL);
		printf("adress of thread after creation %x \n",&z);
		printf("JOINED\n");
		int holder;
		int *x = &holder;
		pthread_join(z,(void **)&x);
		printf("%d\n",holder);
	//	printf("Back in main bois");
}

