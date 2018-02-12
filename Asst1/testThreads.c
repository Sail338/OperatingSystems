#include "my_pthread_t.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
my_pthread_t * list;
int testFunc(void *  val){
    char fileName[50];
    sprintf(fileName,"tests/test%d.txt",*((int*)val));
    FILE * fp = fopen(fileName,"a");
    int i;
    for(i = 0; i < 200000; i++){
        fprintf(fp,"IN THREAD %d: Current Val: %d\n",*((int*)val),i);
    }
    if(*((int*)val) >= 100){
        int v = *((int*)val);
        int z;
        for(z = v-100; z < v; z++){
            my_pthread_join(list[z],NULL);
        }
    }
    fprintf(fp,"I WAITED ON THE LAST 100 THREADS!\n");
    fclose(fp);
    return 0;
}

my_pthread_t * testThreads(int num){
    list = malloc(sizeof(my_pthread_t)*num);
    int i;
    for(i = 0; i < num; i++){
        int * z = malloc(sizeof(int)*num);
        z[i] = i;
        my_pthread_create(&list[i], NULL,(void*)testFunc,&z[i]);
    }
    return list;
}

int main(){
        clock_t begin = clock();
        int size = 150;
        list = testThreads(size);
		//printf("adress before create %x\n",&z);
        //my_pthread_create(&z,NULL,(void*)func1,NULL);
		//printf("adress of thread after creation %x \n",&z);
		//printf("JOINED\n");
		//int *x;
		//my_pthread_join(z,(void **)&x);
		//printf("%d\n",*x);
		//printf("Back in main bois");
        //
        //Main Thread should run super long as to not end the process
        int i = 0;
        int * x;
        while( i < size){
            my_pthread_join(list[i],(void**)&x);
            i+=1;
        }
        clock_t end = clock();
        double totTime = (double)(end-begin)/CLOCKS_PER_SEC;
        printf("FIN\nTIME: %f\n",totTime);
        return 0;
}

