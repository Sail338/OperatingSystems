
#include "my_pthread_t.h"

int main(){
	char* x = mymalloc(8188);
	x[0] = '1';
	x[1] = '2';
	x[2] = '\0';
	printf("%s\n",x);


}
