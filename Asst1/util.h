
#ifndef UTIL_H
#define UTIL_H
#include <ucontext.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LEVELS 10;
int MULTIPLIER = 2;
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

typedef struct Scheduler{
    threadQ ** tq;
    threadNode* current;
    int no_threads;
}Scheduler;
 



Scheduler* scheduler = NULL;
void enqueue(threadNode *);
threadNode* dequeue();
void _thread_q_init(threadNode *,threadQ* );
#endif
