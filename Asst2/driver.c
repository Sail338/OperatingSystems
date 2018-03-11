#include "my_pthread_t.h"

int main(){
	char *x = osmalloc(3);
	*x = 'a';
	printf("x is %s",x);


}
