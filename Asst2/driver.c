#include "my_pthread_t.h"

int main(){
	int *x = (int *)mymalloc(4);
	*x = 1;
	int * y = (int *) mymalloc(4);
	*y = 2;
	printf("x contents: %d, y contents: %d\n", *x, *y);

}
