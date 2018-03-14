#include "my_pthread_t.h"

int main(){
	int *x = (int *)mymalloc(1360);
	*x = 1;
	int *y = (int*)mymalloc(1360);	
	*y = 2;
	int *z =(int*)mymalloc(1360);
	*z = 3;
//	printf("FREEING Y\n");
	my_free(y);
//	printf("FREEING Z\n");
	my_free(z);
	int *a = mymalloc(2720);
	*a =77;
	int *b = mymalloc(1000);
	*b = 69;
	printf("x add %p a address %p b addr %p \n",x,a, b);
}
