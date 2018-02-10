#include "my_pthread_t.h"
#include <unistd.h>
my_pthread_t waitForMe;
int func(int x){
	printf("new thread created\n");
	printf("HELLO WORLD\n");
	printf("KEK\n");
	printf("x is %d\n",x);
    int i = 0;
    while(i < 2000000000){printf("IM THREAD1\n");}
	//swapcontext(&(scheduler->tq[0]->front->thread),&(scheduler->current->thread));
    my_pthread_exit(NULL);
	return  0;

}
int func1(){
    printf("Currently in second thread\n");
	my_pthread_yield();
	int *x = malloc(sizeof(int));
	*x = 9;
	printf("prepare to exit on the right \n");
	my_pthread_exit(x);
    return 0;
}

int waitForMePlease(){
    int z;
    int y;
    for(y = 0; y < 30;y++){
        for(z= 0; z < 20000000; z++){
            continue;
        }
    }
    printf("Finished with the wait: Sending back 10 to all that are joining!\n");
    int* x = malloc(sizeof(int));
    *x = 10;
    my_pthread_exit(x);
    return 0;
}

int testFunc(void *  val){
    int * x;
    printf("In THREAD %d\n",scheduler->current->tid);
    my_pthread_join(waitForMe,(void**)&x);
    if(x == NULL){
        printf("THREAD TERMINATED!\t%x\n",*((int*)val));
    }
    else{
        printf("Thread Number and Ret:\t%d\t%d\n",*((int*)val),*x);
    }
    //my_pthread_exit(NULL);
    return 0;
}

int testThreads(int num){
    my_pthread_create(&waitForMe,NULL,(void*)waitForMePlease,NULL);
    my_pthread_t * list = malloc(sizeof(my_pthread_t)*num);
    int i;
    for(i = 0; i < num; i++){
        int * z = malloc(sizeof(int)*num);
        z[i] = i;
        my_pthread_create(&list[i], NULL,(void*)testFunc,&z[i]);
    }
    return 0;
}

int main(){

        testThreads(5000);
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
        int *x;
        my_pthread_join(waitForMe,(void**)&x);
        printf("Hey I just finished Join from Main Context!\n");
        return 0;
}

