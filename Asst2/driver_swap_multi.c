#include "my_pthread_t.h"

int thread();

int main()

{
	int i;
	char *x[NUM_PAGES];
	for(i=0;i<NUM_PAGES;i++){
        if(i%2 == 0){
            x[i] = mymalloc(3000);
        }
        else
        {
		    x[i] = mymalloc(8000);
            i += 1;
        }
	}
	for(i=0;i<NUM_PAGES;i++){
		if(i%2 == 0 && x[i] != NULL){
			my_free(x[i]);
		}
        else{
            i+=1;
        }
	}
    page_table_string(0,4);
    x[1][0] = 'h';
    x[1][4098] = 's';
	pthread_t t;
	pthread_create(&t,NULL,(void*)thread,NULL);
	pthread_join(t,NULL);
    page_table_string(0,4);
	printf("%c\n%c\n",x[1][0],x[1][4098]);
}
int thread(){
	printf("Swapped in\n");	
	char *y = mymalloc(12000);
	printf("address of y is %p\n",y);
    page_table_string(0,4);
	pthread_exit(NULL);
	
}
