#include "my_pthread_t.h"
#include <time.h>
#include <stdlib.h>
int  masterState;
int  next;

void func(int * x)
{
    srand(time(NULL));
    int iterations = rand()+1; 
    while(iterations > 0)
    {
        srand(time(NULL));
        int mallocSpace = rand()%1000;
        printf("THREAD %d | ITER %d:\tTRYING TO MALLOC SPACE: %d\n",*x,iterations,mallocSpace);
        printf("THREAD %d | ITER %d:\tFREE RAM: %d   FREE SWAP: %d\n",*x,iterations,PT->free_pages_in_RAM,PT->free_pages_in_swap);
        char * z = malloc(mallocSpace);
        if(z == NULL)
        {
            printf("THREAD %d | ITER %d:\tMALLOC RETURNED A NULL POINTER\n",*x,iterations);
            return;
        }
        else
        {
            printf("THREAD %d | ITER %d:\tMALLOC RETURNED A VALID POINTER: %p\n",*x,iterations,z-4);
            *z = 'S';
            //Yield implicitly so that hopefully another thread swaps out this threads data
            //and the print statement below causes another swap back
            my_pthread_yield();
            printf("THREAD %d | ITER %d:\tVALUE IN *Z = %c\n",*x,iterations,*z);
            bool statusFree = my_free(z);
            if(statusFree == true)
            {
                printf("THREAD %d | ITER %d:\tFREED WITHOUT ERROR: FREE RAM: %d  FREE SWAP: %d\n",
                        *x,iterations,PT->free_pages_in_RAM, PT->free_pages_in_swap);
            }
            else
            {
                printf("THREAD %d | ITER %d:\tFREED FAILED?\n",*x,iterations);
            }

        }
        iterations -= 1;
    }
    return;
}

int main()
{
    //masterState = malloc(sizeof(int));
    //next = malloc(sizeof(int));
    masterState = 0;
    next = 0;
    char * x = malloc(sysconf(_SC_PAGE_SIZE)*NUM_PAGES-4);
    memcpy(x,"Welcome to the Jungle!",23);
    pthread_t * t = malloc(sizeof(my_pthread_t)*20);
    int j = 0;
    int * z = malloc(20*sizeof(int));
    for(j = 0; j < 20; j++)
    {
        z[j] = j;
        pthread_create(&t[j],NULL,(void*)func,&z[j]);
    }
    for(j = 0; j < 20; j++)
    {
        pthread_join(t[j],NULL);
    }
    page_table_string(0,10);
}
