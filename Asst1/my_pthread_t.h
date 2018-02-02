//File: my_pthread_t.h
//Author: Yujie REn
//Date: 09/23/2017

//name: SaraAnn Stanway, Srihari Shankar, Sam Azouzi
//username of iLab: sys41
//iLab server: template

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <ucontext.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef uint my_pthread_t;

typedef struct threadControlBlock {
	/* add something here */
} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
} my_pthread_mutex_t;

/* define your data structures here: */
typedef struct threadNode
{
	struct threadNode * next;
	ucontext_t thread;
	double spawnTime;
    uint tid;
    int qlevel;
} threadNode;
/**
 * typedef struct BiggerStruct{
 *    MultiQ * List;
 *    threadNode * Current;
 *    Hashtable of all threads with their thread ids
 *    Number of threads currently running
 * }
 * 
 * 
 * 
**/
typedef struct Scheduler{
    threadQ ** tq;
    threadNode* current;
    int no_threads;
}Scheduler;
 


typedef struct threadQ{
    threadNode * front;
    threadNode * rear;
    int size;
    double min;
    double max;
    int upjmp;
    int dwnjmp;
} threadQ;

Scheduler* scheduler = NULL;
/**
 *Assuming we update the level beforehand
 * Enqueues onto a level
 *
 **/
void enqueue(threadNode *Node){
    //check if the head if the queue is null
    threadQ* threadq = scheduler->tq[Node->qlevel];
    if(threadq == NULL){
        threadq = malloc(sizeof(threadQ));
        _enqueueHelper(Node,threadq);
        threadq ->size =1;
        
    } 
    //front is empty similar if the list has not been malloced yet
    else if (threadq -> front == NULL){
        _enqueueHelper(Node,threadq);
        threadq ->size ++;
    } 
    // add to the end of the Linked list
    else{
        threadq ->rear->next =(threadNode *) malloc(sizeof(threadNode));
        threadq ->rear ->next = Node;
        threadq->rear = Node;
        threadq -> rear ->next = NULL;
        threadq ->size ++;
    }

}

void _enqueueHelper(threadNode *Node,threadQ* threadq){
        threadq -> front = (threadNode *)malloc(sizeof(threadNode));
        threadq ->rear = (threadNode *)malloc(sizeof(threadNode));
        threadq ->front = Node;
        threadq ->rear = Node;
        threadq ->front ->next = threadq->rear;
        threadq -> rear ->next = NULL;


}












// Feel free to add your own auxiliary data structures


/* Function Declarations: */

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif

