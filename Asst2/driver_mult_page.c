
#include "my_pthread_t.h"
#include <string.h>

int main(){
		char *x = mymalloc(4098);
		char *y = mymalloc(10);
		my_free(y);
		printf("DRIVER MALLOC%d\n",*(int *)((x+4098)));
}
