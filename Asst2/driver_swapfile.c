#include "my_pthread_t.h"



void func()
{
    char * y = mymalloc(4000);
    char * z = mymalloc(4000);
    char * d = mymalloc(4000);
    page_table_string(0,4);
    return;
}

int main()
{
    char * x = mymalloc(7340032-8);
    x[8100] = 's';
    pthread_t t;
    pthread_create(&t,NULL,(void*)func,NULL);
    pthread_join(t,NULL);
    printf("%c\n",x[8100]);
 
    page_table_string(0,4);
}
