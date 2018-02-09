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
#define LEVELS 10
#define MULTIPLIER 2
//Has to keep in mind how many levels in the Scheduler
#define MAXTHD 10
/* define your data structures here: */
typedef struct threadNode
{
	struct threadNode * next;
	ucontext_t thread;
	double spawnTime;
    uint tid;
    int qlevel;
	//for prioritization of old threads. when something gets pushed down in the MLPQ, increase this
	//so that when it comes back to the top after a while, it gets to run for longer
	int numSlices;
	bool yield;
} threadNode;

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

}Scheduler;
 

void enqueue(threadNode *);
threadQ * get_next_executable(int * curr);
int init;
Scheduler* scheduler;
threadQ* _scan_non_empty(int*curr);
threadNode* dequeue();
threadNode* mutex_dequeue(my_pthread_mutex_t *);
void thread_q_init(threadNode *,threadQ*);
#endif
