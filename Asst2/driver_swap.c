#include "my_pthread_t.h"

int thread();

int main()

{
	int i;
	char *x[NUM_PAGES];
	for(i=0;i<NUM_PAGES;i++){
		x[i] = mymalloc(3000);
	}
	for(i=0;i<NUM_PAGES;i++){
		if(i%2 == 0){
			my_free(x[i]);
		}
	}

	pthread_t t;
	pthread_create(&t,NULL,(void*)thread,NULL);
	pthread_join(t,NULL);
    page_table_string(0,4);
	printf("%p",x[1]);
}
int thread(){
	printf("Swapped in\n");	
	char *y = mymalloc(8000);
	printf("address of y is %p",y);
//	page_table_string(0,10);
	pthread_exit(NULL);
	
}
