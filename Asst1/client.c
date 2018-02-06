#include "my_pthread_t.h"
#include <unistd.h>
int func(){
	printf("new thread created\n");
	printf("HELLO WORLD\n");
	printf("KEK\n");
	setcontext(&(scheduler->current->thread));
	return  0;

}

int main(){
		my_pthread_t t;
		my_pthread_create(&t,NULL,(void*)func,NULL);
}

