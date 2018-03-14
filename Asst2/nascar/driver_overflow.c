#include "../my_pthread_t.h"


int main()
{
	char *x = mymalloc(8200);
	char*y = mymalloc(10);
	page_table_string(0,3);
	my_free(x);
	char*z = mymalloc(1);


	page_table_string(0,5);

	printf("z addr is %p\n",z);
	printf("y addr is %p\n",y);


}
