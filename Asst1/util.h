
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
#define LEVELS 10
#define MULTIPLIER 2
/* define your data structures here: */
typedef struct threadNode
{
	struct threadNode * next;
	ucontext_t thread;
	double spawnTime;
    uint tid;
    int qlevel;
} threadNode;

typedef struct threadQ{
    threadNode * front;
    threadNode * rear;
    int size;
    double min;
    double max;
    int upjmp;
    int dwnjmp;
} threadQ;

//mutex struct definition
typedef struct my_pthread_mutex_t
{
	bool isLocked;
	threadQ * waitQ;
	//may want to also include pointer to currently executing thread
} my_pthread_mutex_t;

void mutex_enqueue(threadNode*, my_pthread_mutex_t *);


typedef struct Scheduler{
    threadQ ** tq;
    threadNode* current;
    int no_threads;
}Scheduler;
 


Scheduler* scheduler;
void enqueue(threadNode *);
threadNode* dequeue();
threadNode* mutex_dequeue(my_pthread_mutex_t *):
void _thread_q_init(threadNode *,threadQ* );
#endif
