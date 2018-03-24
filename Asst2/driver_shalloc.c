#include "my_pthread_t.h"

int thread();

int main()

{
	int i;
	char *x = shalloc(16362);
    char *a = shalloc(2);
    char *b = shalloc(2);
    char *c = shalloc(2);
    my_free(a);
    my_free(b);
    my_free(c);
    memcpy(x,"Hi",3);
	printf("%s\n",x);
    pthread_t t;
	pthread_create(&t,NULL,(void*)thread,NULL);
	pthread_join(t,NULL);
    printf("%s\n",x);
    //page_table_string(0,4);
}
int thread(){
    char * z = shalloc(14);
    if(z != NULL)
    {
        memcpy(z,"Si",3);
        printf("%s\n",z);
    }
    return 0;
}
