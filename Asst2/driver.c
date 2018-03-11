#include "my_pthread_t.h"

int main(){
	int *x = osmalloc(3);
	*x = 3;
	printf("x is %d",*x);


}
