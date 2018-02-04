//File: my_pthread.c
//Author: Yujie REN
//Date: 09/23/2017

//name: SaraAnn Stanway, Sam Azouzi, Srihari Shankar
//iLab Server: template
#include "my_pthread_t.h"
#include <sys/time.h>
#include <ucontext.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#define MEM 64000

void sig_hanlder(){
	printf("%s","I am a sig handler");

}
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	/**
	 *
	 *Initlaize scheduler
	 **/
	if(init == 0){
		scheduler = (Scheduler *)malloc(sizeof(Scheduler));
		scheduler->tq = (threadQ **)malloc(sizeof(threadQ *) * LEVELS);
		int i;
		for(i=0;i<LEVELS;i++){

			scheduler->tq[i] = NULL;
		}
		scheduler -> current = NULL;
		scheduler -> no_threads = 1;
		memset(&(scheduler->sa),0,sizeof(scheduler->sa));
		scheduler ->sa.sa_handler = &sig_hanlder;
		sigaction(SIGVTALRM,&scheduler->sa,NULL);
		scheduler->timer.it_value.tv_sec = 0;
		scheduler -> timer.it_value.tv_usec = 25000;
		scheduler ->timer.it_interval.tv_sec = 0;
		scheduler ->timer.it_interval.tv_usec = 25000;
		

		
	}
	//create a threadNode
	threadNode * node;
	node = malloc(sizeof(threadNode));
	node -> tid = &node;
	*thread = node ->tid;
	node -> next = NULL;
	node -> spawnTime = (double)time(NULL); 
	node -> numSlices = 25;
	node -> qlevel = 0;
	ucontext_t curr;
	ucontext_t newthread;
	getcontext(&curr);
	getcontext(&newthread);
	//we dont want anything to happen after thread is done
	newthread.uc_link = 0;
	newthread.uc_stack.ss_sp=malloc(MEM);
	newthread.uc_stack.ss_size=MEM;
 	newthread.uc_stack.ss_flags=0;
	node ->thread = malloc(sizeof(ucontext_t));
	printf("finished makeing node \n");
	makecontext(&newthread, (void*)(function), 1,arg);
    node->thread = &newthread;
	enqueue(node);
    swapcontext(&curr,&newthread);
	/**
	 *
	 *We have init check twice because we want to start the timer AFTER the node has been created and enqueued
	 * **/
	if(init == 0){
			init = 1;
			//setitimer(ITIMER_VIRTUAL,&scheduler->timer,NULL);	
			//start timer
			//
		}
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {

	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	mutex = (my_pthread_mutex_t *)malloc(sizeof(my_pthread_mutex_t));
	//if not enough memory to alloc for new mutex
	if (mutex)
	{
		mutex -> isLocked = false;
		mutex -> waitQ = NULL;
		return 0;
	}
	return -1;
	//ask Franny about EBUSY
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};



