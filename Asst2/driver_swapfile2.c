#include "my_pthread_t.h"

int  masterState;
int  next;

void func()
{
    char * y = mymalloc(8000);
    char * z = mymalloc(8000);
    char * d = mymalloc(8000);
    memcpy(y,"Hello y",8);
    memcpy(z,"Hello z",8);
    memcpy(d,"Hello d",8);
    printf("PAGE TABLE SHOULD HAVE CONSECUTIVE DOUBLE PAGES THAT BELONG TO THREAD!\n");
    page_table_string(0,10);
    __atomic_store_n(&next,1,__ATOMIC_SEQ_CST);
    while(masterState == 0);
    printf("%s\n",y);
    printf("%s\n",z);
    printf("%s\n",d);
    printf("THREAD ACCESSES DATA THAT WAS SWAPPED BY MAIN!\n");
    page_table_string(0,10);
     __atomic_store_n(&next,0,__ATOMIC_SEQ_CST);
    return;
}

int main()
{
    //masterState = malloc(sizeof(int));
    //next = malloc(sizeof(int));
    masterState = 0;
    next = 0;
    char * x = mymalloc(7340032-8);
    x[8100] = 's';
    pthread_t t;
    pthread_create(&t,NULL,(void*)func,NULL);
    int i = 0;
    while(next == 0);
    printf("%c\n",x[8100]);
    masterState = 1;
    __atomic_store_n(&masterState,1,__ATOMIC_SEQ_CST);
    while(next == 1);
    printf("THE PAGE_TABLE SHOULD HAVE NOT CHANGED BECAUSE MAIN NEVER ACCESSES ITS DATA AFTER MASTER STATE SET!\n");
    page_table_string(0,10);
}
