#include "../my_pthread_t.h"


int main()
{
	char *x = mymalloc(8200);
	char*y = mymalloc(10);
	page_table_string(0,3);
	my_free(x);
	char*z = mymalloc(4);


	page_table_string(0,5);

	printf("y addr is %p\n. z address is %p, z metadata is %d\n, leftover metadata: %d\n",y-4, z-4, *(int *)(z-4), *(int *)(z+4));


}
