#include "my_pthread_t.h"

int main(){
	int *x = (int *)osmalloc(OSLAND-10);
	*x = 1;
	int * y = (int *) osmalloc(1);
	*y = 2;
	printf("x contents: %d, y contents: %d\n", *x, *y);
}
