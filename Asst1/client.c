#include "my_pthread_t.h"
#include <unistd.h>
int func(int x){
	printf("new thread created\n");
	printf("HELLO WORLD\n");
	printf("KEK\n");
	printf("x is %d\n",x);
	swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
	return  0;

}

int main(){
		my_pthread_t t;
		typedef struct args {
			int arg1;
			int arg2;

		}args;
		args arg;
	   arg.arg1 = 5;
   	    arg . arg2 = 6;	   
		my_pthread_create(&t,NULL,(void*)func,(void *)5);
		printf("Back in main bois");
}

