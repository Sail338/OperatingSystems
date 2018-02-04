#include "my_pthread_t.h"

int func(){
	printf("new thread created");
	return  0;

}
int main(){
		my_pthread_t t;
		my_pthread_create(&t,NULL,(void*)func,NULL);
}

