#include "my_pthread_t.h"
#include <unistd.h>
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
	int *x = malloc(sizeof(int));
	*x = 9;
	printf("prepare to exit on the right \n");
	my_pthread_exit(x);
    return 0;
}


int main(){
        my_pthread_t z;
        my_pthread_create(&z,NULL,(void*)func1,NULL);
		int *holder;
		my_pthread_join(z,(void **)&holder);
		printf("%d\n",*holder);
        printf("HAAHAHAHAHAHAHA2\n");
	//	printf("Back in main bois");
}

