#ifndef UTIL_H
#define UTIL_H
#include <ucontext.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#define LEVELS  5
#define MULTIPLIER 2
//Has to keep in mind how many levels in the Scheduler
#define MAXTHD 10
#define pthread_create my_pthread_create
#define pthread_yield my_pthread_yield
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define MEM 64000
/* define your data structures here: */
 typedef struct threadControlBlock {
	struct threadControlBlock * next;
	ucontext_t thread;
	double spawnTime;
    uint tid;
    int qlevel;
    struct threadControlBlock * waitingNodes;
	//for prioritization of old threads. when something gets pushed down in the MLPQ, increase this
	//so that when it comes back to the top after a while, it gets to run for longer
	int numSlices;
	void * return_value;
    int term;
	//flag to detect whether this tread joined another thread or not
	bool did_join;
	bool is_waiting;
	 } tcb;
typedef tcb  threadNode;
typedef threadNode * my_pthread_t;
typedef struct threadQ
{
    threadNode * front;
    threadNode * rear;
    int size;
    int threshold;
    double min;
    double max;
} threadQ;

//mutex struct definition
typedef struct my_pthread_mutex_t
{
	bool isLocked;
	threadQ * waitQ;
	threadNode * currThread;
	//may want to also include pointer to currently executing thread
} my_pthread_mutex_t;

void mutex_enqueue(threadNode*, my_pthread_mutex_t *);

typedef struct Scheduler
{
    threadQ ** tq;
    threadNode* current;
    int no_threads;
	struct sigaction sa;
    struct itimerval timer;
    bool SYS;

}Scheduler;
void schedulerString(); 
void * wrapper_function(void*(*start)(void*), void*args);
void enqueue(threadNode *);
threadQ * get_next_executable(int * curr);
int init;
Scheduler* scheduler;
threadQ* _scan_non_empty(int*curr);
threadNode* dequeue();
threadNode* mutex_dequeue(my_pthread_mutex_t *);
void thread_q_init(threadNode *,threadQ*);
void yield_sig_handler(int signum);
void normal_sig_handler();
threadNode* createNewNode(threadNode*,int,int,double,my_pthread_t *,void*(*function)(void*),void*);
int initScheduler();
#endif
