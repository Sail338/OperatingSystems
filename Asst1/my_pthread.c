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
#include <errno.h>
#define MEM 64000
int tCount=0;

//this is scheduled to run every 25 milliseconds
void normal_sig_handler()
{
	
}

void yield_sig_handler(int signum)
{
	// RESET the timer to 0
  setitimer(ITIMER_VIRTUAL,0,NULL);
  //set a temp node to current which is context that we ARE GOING TO SWAP OUT
  threadNode * temp = scheduler -> current;
  
  threadNode * dequeuedNode = NULL;
  //enqueue the current Node back to the MLQ
  enqueue(scheduler -> current);
  //DEQUEUE a Node
  dequeuedNode = dequeue((scheduler -> current->qlevel));
  //set the current equal to the dqed Node
  scheduler -> current = dequeuedNode;
  //reset the timer
  setitimer(ITIMER_VIRTUAL, &(scheduler->timer), NULL);
 //swap th contexts 
 swapcontext(&(temp->thread),&(scheduler -> current->thread));
  
}

threadNode * createNewNode(threadNode *node,int level,int numSlices,double spawnTime,my_pthread_t *thread,void*(*function)(void*),void * arg)
{
	ucontext_t newthread;
	node = (threadNode *)malloc(sizeof(threadNode));
	node -> tid = tCount++;
	node -> next = NULL;
	node -> spawnTime = spawnTime; 
	node -> numSlices = numSlices;
	node -> qlevel = level;
	node -> yield = false;
	/**
	 * IF the argument is NULL that means we already have a context with a function, we dont have to call makecontext(), we use when we want to create a Node for the main thread
	 * **/

	getcontext(&newthread);
	//only create a new context if we pass in a new parameter
	if(function != NULL){

		thread = node;
	//we dont want anything to happen after thread is done
		newthread . uc_link = 0;
		newthread. uc_stack.ss_sp=malloc(MEM);
		newthread. uc_stack.ss_size=MEM;
 		newthread. uc_stack.ss_flags=0;
		makecontext(&newthread, (void*)(function), 1,arg);
	}
	
	//node ->thread = malloc(sizeof(ucontext_t*));
	
    node->thread = newthread;
	return node;
	
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) 
{
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
		//memset fails
		scheduler ->sa.sa_handler = &normal_sig_handler;
		sigaction(SIGVTALRM,&scheduler->sa,NULL);
		scheduler->timer.it_value.tv_sec = 0;
		scheduler -> timer.it_value.tv_usec = 25000;
		scheduler->current = NULL;
		scheduler->current = createNewNode(scheduler->current,0,25,(double)time(NULL),NULL,NULL,NULL);
			
		
	}
	//create a threadNode
	threadNode * node = NULL;
	node = createNewNode(node,0,25,(double)time(NULL),thread,function,arg);
	enqueue(node);
	my_pthread_yield();
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
	/**
	 *if anything fails (aka errno not 0) return -1
	 *
	 **/
	if(errno !=0){
		return -1;
	}
	return 0;
};

//YIELD WILL NUDGE THE SIGNAL HANDLER TO STEP DOWN THE CURRENT THREAD
//THE NEXT TIME IT GOES OFF
/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() 
{
	printf("called yield");	
	//calls signal handler
	signal(SIGUSR1, yield_sig_handler);
	raise(SIGUSR1);
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) 
{
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) 
{
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) 
{
	mutex = (my_pthread_mutex_t *)malloc(sizeof(my_pthread_mutex_t));
	//if not enough memory to alloc for new mutex
	if (mutex)
	{
		mutex -> isLocked = false;
		mutex -> waitQ = NULL;
		return 0;
	}
	return -1;
	//TODO ask Franny about EBUSY
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t * mutex) 
{
	bool lock_state = __sync_or_and_fetch (&mutex, true);
	if (lock_state == true)
	{
		//save the context
		//set scheduler context to null
		//yield
		mutex_enqueue(scheduler->current, mutex);
		getcontext(&scheduler->current->thread);
		scheduler->current = NULL;
		my_pthread_yield();
		return -1;
	}
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t * mutex) 
{
	if (mutex -> isLocked == false)
	{
		return -1;
	}
	else
	{
		mutex -> isLocked = false;
		if (mutex -> waitQ != NULL)
		{
			threadNode * curr = mutex_dequeue(mutex);
			if (curr != NULL)
			{
				enqueue(curr);
			}
		}
	}
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) 
{
	return 0;
};



