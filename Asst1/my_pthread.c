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
void normal_sig_handler(int signum)
{
    if(__atomic_load_n(&(scheduler->SYS),__ATOMIC_SEQ_CST)){
        return;
    }
	setitimer(ITIMER_VIRTUAL, 0, NULL);
	threadNode * curr = scheduler->current;
	curr->numSlices --;
	if (curr->numSlices == 0)
	{
		curr->qlevel = (curr->qlevel+1)%LEVELS;
		curr->numSlices = curr->qlevel+1;
        //schedulerString();
		yield_sig_handler(3);
	}
    else{
        setitimer(ITIMER_VIRTUAL,&scheduler->timer,NULL);
    }

}

void * wrapper_function(void*(*start)(void*),void*args){
    start(args);
    my_pthread_exit(NULL);
}




void schedulerString(){
    printf("Current Context Address %x\n",scheduler->current);
    printf("Current QLevel: %d\n",scheduler->current->qlevel);
    //Print out each Queue
    int x;
    for(x = 0; x < LEVELS; x++){
        if(scheduler->tq[x] == NULL){
            break;
        }
        threadNode * ptr = scheduler->tq[x]->front;
        printf("Queue %d:\nSIZE:%d\t\n",x,scheduler->tq[x]->size);
        while(ptr != NULL){
            printf("%x\t",ptr);
            ptr = ptr->next;
        }
        printf("NULL\n\n");
    }
}


void yield_sig_handler(int signum)
{
	// RESET the timer to 0
   setitimer(ITIMER_VIRTUAL,0,NULL);
   //schedulerString();
  //set a temp node to current which is context that we ARE GOING TO SWAP OUT
  threadNode * temp = scheduler -> current;
  if(temp == NULL){
    temp = dequeue(0);
    scheduler->current = temp;
    setitimer(ITIMER_VIRTUAL, &(scheduler->timer), NULL);
    setcontext(&(scheduler->current->thread));
  }
  else{
    threadNode * dequeuedNode = NULL;
    //enqueue the current Node back to the MLQ
	if(scheduler->current->did_join == false &&  scheduler->current->is_waiting == false){
    	enqueue(scheduler -> current);
	}
    //DEQUEUE a Node
    //dequeuedNode = dequeue((scheduler ->current->qlevel)); GOING TO DEQUEUE 0
    dequeuedNode = dequeue(0);
    if(dequeuedNode == NULL){
        return;
    }
    //set the current equal to the dqed Node
    scheduler -> current = dequeuedNode;
    scheduler->current->next = NULL;
    //reset the timer
    setitimer(ITIMER_VIRTUAL, &(scheduler->timer), NULL);
    //swap th contexts
    swapcontext(&(temp->thread),&(scheduler -> current->thread));
    
  }
}

threadNode * createNewNode(threadNode *node,int level,int numSlices,double spawnTime,my_pthread_t *thread,void*(*function)(void*),void * arg)
{
	ucontext_t newthread;
	//thread = malloc(sizeof(my_pthread_t));
	node = (threadNode *)malloc(sizeof(threadNode));
	node -> tid = tCount++;
	node -> next = NULL;
	node -> spawnTime = spawnTime; 
	node -> numSlices = numSlices;
	node -> qlevel = level;
	node -> waitingNodes = NULL;
	node -> term =0;
    node -> return_value = NULL;
	node->did_join = false;
	node->is_waiting = false;
	/**
	 * IF the argument is NULL that means we already have a context with a function, we dont have to call makecontext(), we use when we want to create a Node for the main thread
	 * **/

	getcontext(&newthread);
	//only create a new context if we pass in a new parameter
	if(function != NULL){
		
		*thread = node;
        
	    //we dont want anything to happen after thread is done
		newthread . uc_link = 0;
		newthread. uc_stack.ss_sp=malloc(MEM);
		newthread. uc_stack.ss_size=MEM;
 		newthread. uc_stack.ss_flags=0;
		makecontext(&newthread,(void*)wrapper_function, 2,function,arg);
		
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
		scheduler->timer.it_value.tv_usec = 25000;
		scheduler->current = NULL;
		scheduler->current = createNewNode(scheduler->current,0,1,(double)time(NULL),NULL,NULL,NULL);
	    scheduler->SYS = false;		
		
	}
	//create a threadNode
	threadNode * node = NULL;
	node = createNewNode(node,0,1,(double)time(NULL),thread,function,arg);
    //thread = (my_pthread_t)node;
	enqueue(node);
	/**
	 *
	 *We have init check twice because we want to start the timer AFTER the node has been created and enqueued
	 * **/
	if(init == 0){
			init = 1;
		    setitimer(ITIMER_VIRTUAL,&scheduler->timer,NULL);	
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
    my_pthread_yield();
	return 0;
};

//YIELD WILL NUDGE THE SIGNAL HANDLER TO STEP DOWN THE CURRENT THREAD
//THE NEXT TIME IT GOES OFF
/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() 
{
	//calls signal handler
	signal(SIGUSR1, yield_sig_handler);
	raise(SIGUSR1);
	return 0;
};

void my_pthread_exit(void *value_ptr) {
    threadNode * toBeDeleted = NULL;
    toBeDeleted = scheduler->current;
    //1. Deal with Waiting
    //2. Call yeild
    //Yield can assume empty current
   // free(toBeDeleted->thread.uc_stack.ss_sp);
    scheduler->current = NULL;
    toBeDeleted->term = 1;
    if(toBeDeleted->waitingNodes != NULL){
        toBeDeleted -> return_value = value_ptr;
        threadNode * nextOne = toBeDeleted->waitingNodes;
        while(nextOne != NULL){
            nextOne = toBeDeleted->waitingNodes->next;
            //printf("%x\n",toBeDeleted->waitingNodes);
            toBeDeleted->waitingNodes->next = NULL;
            enqueue(toBeDeleted->waitingNodes);
            toBeDeleted->waitingNodes = nextOne;
		
        }
    }
    my_pthread_yield();
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    __atomic_store_n(&(scheduler->SYS),true, __ATOMIC_SEQ_CST);
	//TODO reset node level to 0
    //We are going to change the current equal to the thread that is being waited on
    threadNode * thJ = thread;
    threadNode * temp = NULL;
		
    if(thJ->term != 1){
        if(thJ->waitingNodes == NULL){
            thJ->waitingNodes = scheduler->current;
            temp = thJ->waitingNodes;
        }
        else{
	
            threadNode * front = thJ->waitingNodes;
            while(front->next != NULL){
                front=front->next;
            }
            front->next = scheduler->current;
            temp = front->next;
        }
       /* scheduler->current = dequeue(scheduler->current->qlevel);
        if(scheduler->current == NULL){
          return -1;       
        }*/
		scheduler->current->did_join = true;
       // getcontext(&(temp->thread));
        __atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
        my_pthread_yield();  //DEBUG: yield is not changing the value of
		//reset to did join back to false
        if(value_ptr != NULL){

	 	    *value_ptr = thread->return_value;   //DEBUG: Return Value is 0 for some reason

        }
        return 0;
    }
	//TODO check Need to deal with errorno
    if(value_ptr != NULL){
        *value_ptr = NULL;
    }
    __atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
	return -1;
    
}


/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) 
{
	mutex = (my_pthread_mutex_t *)malloc(sizeof(my_pthread_mutex_t));
	//if not enough memory to alloc for new mutex
	if (mutex)
	{
		mutex -> isLocked = false;
		mutex -> waitQ = NULL;
		mutex -> currThread = NULL;
		return 0;
	}
	return -1;
	//TODO ask Franny about EBUSY
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t * mutex) 
{
	
    __atomic_store_n(&(scheduler->SYS),true, __ATOMIC_SEQ_CST);
	//returns true only if contents are already locked
	if (__atomic_test_and_set(&(mutex->isLocked), __ATOMIC_SEQ_CST) )
	{
		//save the context
		//set scheduler context to null
		//yield
		//
		mutex_enqueue(scheduler->current, mutex);
		scheduler->current->is_waiting = true;
		printf("failed to lock\n");
    	__atomic_store_n(&(scheduler->SYS),false, __ATOMIC_SEQ_CST);
		my_pthread_yield();
		return -1;
	}
	//is the pointer arithmetic right? p sure it is just want to double check
	mutex -> currThread = scheduler->current;
    __atomic_store_n(&(scheduler->SYS),false, __ATOMIC_SEQ_CST);
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t * mutex) 
{
	
    __atomic_store_n(&(scheduler->SYS),true, __ATOMIC_SEQ_CST);
	//NEED TO PUT THIS IN SYS MODE
	//in case mutex wasn't actually locked before, or the thread trying to unlock it isn't the one that set the lock
	if (mutex->currThread != scheduler->current)
	{
    	__atomic_store_n(&(scheduler->SYS), false, __ATOMIC_SEQ_CST);
		return -1;
	}
	else
	{
		if (mutex -> waitQ != NULL)
		{
			threadNode * curr = mutex_dequeue(mutex);
			if (curr != NULL)
			{
				curr -> next = NULL;
				curr -> is_waiting = false;
				mutex -> currThread = curr;
				scheduler -> current = curr;

			    __atomic_store_n(&(scheduler->SYS),false, __ATOMIC_SEQ_CST);
				my_pthread_yield();
			}
			else
			{
				__atomic_test_and_set(&(mutex->isLocked), __ATOMIC_SEQ_CST);
				mutex->currThread = NULL;
			}
		}
		else
		{
			__atomic_test_and_set(&(mutex->isLocked), __ATOMIC_SEQ_CST);
		}
	}

    __atomic_store_n(&(scheduler->SYS),false, __ATOMIC_SEQ_CST);
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) 
{
	return 0;
};



