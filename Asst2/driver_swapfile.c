#include "my_pthread_t.h"



void func()
{
    char * y = mymalloc(4000);
    //page_table_string(0,4);
    return;
}

int main()
{
    char * x = mymalloc(7884800-8);
    char buffer[300];
    char * y = buffer;
    memcpy(y,"Hey",4);
    printf("%s\n",y);
    pthread_t t;
    //pthread_create(&t,NULL,(void*)func,NULL);
    //pthread_join(t,NULL);
}
