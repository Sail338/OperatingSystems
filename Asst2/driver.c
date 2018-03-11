#include "my_pthread_t.h"

int main(){
	int *x = (int *)osmalloc(3);
	*x = 3;
	printf("My name is is %d",*x);


}
