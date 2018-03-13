
#include "my_pthread_t.h"

int main(){
	char * x = mymalloc(8188);
	x[1] = 'h';
	printf("%s",x);


}
