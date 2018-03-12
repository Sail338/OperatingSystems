#include "my_pthread_t.h"

int main(){
	int *x = (int *)mymalloc(1360);
	int *y = (int*)mymalloc(1360);	
	int *z =(int*)mymalloc(1360);
	my_free(y);
	my_free(z);
	int *test = mymalloc(2720);
	*test =10;

	printf("x add %p y addr %p z addr %p test adress %p \n",x,y,z,test);
	printf("TEST VALUE is %d",*test);
}
