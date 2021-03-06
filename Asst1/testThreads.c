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
    if(*((int*)val) > 100){
        int v = *((int*)val);
        int z;
        for(z = v-100; z < v; z++){
            my_pthread_join(list[z],NULL);
            fprintf(fp,"Finished Joing Thread: %d\n",z);
        }
        fprintf(fp,"I WAITED ON THE LAST 100 THREADS!\n");
    }
    fclose(fp);
    printf("FINISHED WITH THREAD: %d\n",*(int*)val);
    return 0;
}


int testFuncSync(void *  val){
    char fileName[50];
    sprintf(fileName,"tests/test%d.txt",*((int*)val));
    FILE * fp = fopen(fileName,"a");
    int i;
    for(i = 0; i < 200000; i++){
        fprintf(fp, "IN SYNC %d: Current Val: %d\n",*((int*)val),i);
    }
    fclose(fp);
    return 0;
}

my_pthread_t * testThreads(int num){
    list = malloc(sizeof(my_pthread_t)*num);
    int i;
    int * z = malloc(sizeof(int)*num*5);
    int nameOfFile = 0;
    for(i = 0; i < num; i++){
        z[i] = nameOfFile;
        my_pthread_create(&list[i], NULL,(void*)testFunc,&z[i]);
        nameOfFile+=1;
    }
    return list;

}

void testSync(int num){
    int i;
    for(i = num; i < num*2; i++){
        int * x = malloc(sizeof(int));
        *x = i;
        testFuncSync((void*)x);
        free(x);
    }
}

int main(){
        clock_t begin = clock();
        int size = 110;
        list = testThreads(size);
        printf("All Threads Created!");
        int i = 0;
        int * x;
        while( i < size){
            my_pthread_join(list[i],(void**)&x);
            i+=1;
        }
        clock_t end = clock();
        double totTime = (double)(end-begin)/CLOCKS_PER_SEC;
        printf("THREAD FIN\nTIME: %f\n\n\n",totTime);
        //begin = clock();
        //testSync(size);
        //end = clock();
        //totTime = (double)(end-begin)/CLOCKS_PER_SEC;
        //printf("SYNC FIN\nTIME: %f\n",totTime);
        return 0;
}

