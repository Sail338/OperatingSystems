#include "my_pthread_t.h"

int thread();

int main()
{
	char *x[NUM_PAGES];
	int i;
	for(i=0;i<NUM_PAGES;i++){
		x[i] = mymalloc(3000);
	}
	for(i=0;i<NUM_PAGES;i++){
		if(i%2 ==0){
			printf("Hello\n");
			my_free(x[i]);

		}
	}
	page_table_string(0,NUM_PAGES);
//	pthread_t t;
//	pthread_create(&t,NULL,(void*)thread,NULL);
//	while(1);
	

}
int thread(){
	char *y = mymalloc(8000);
	page_table_string(0,NUM_PAGES);
	
}
