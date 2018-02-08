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
int func1(int x){
    printf("Currently in second thread\n");
    int i = 0;
    while(i < 2000000000){printf("IM THREAD2\n");}
    printf("x is %d\n",x);
    my_pthread_exit(NULL);
    return 0;
}


int main(){
		my_pthread_t t;
        my_pthread_t z;
		typedef struct args {
			int arg1;
			int arg2;

		}args;
		args arg;
	   arg.arg1 = 5;
   	    arg . arg2 = 6;
        my_pthread_create(&z,NULL,(void*)func1,(void*)9);
		my_pthread_create(&t,NULL,(void*)func,(void *)5);
        printf("HAAHAHAHAHAHAHA1\n");
        int i = 0;
        while(i<2000000000){printf("IM MAIN!\n");}
        printf("HAAHAHAHAHAHAHA2\n");
	//	printf("Back in main bois");
}

