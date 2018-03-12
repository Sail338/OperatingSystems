#ifndef UTIL_H
#define UTIL_H
#include <malloc.h>
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

//#define malloc(x) mymalloc (x, __FILE__, __LINE__)
#define free(x) osfree(x, __FILE__, __LINE__)
#define OSLAND 503808
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
#define MEM 4096

typedef struct page
{
	//for contigous pages	
	struct page * next_page;
	struct page * prev_page;
	//for the linkedlist
	struct page * next_page_in_ll;
	//data type? hello?
	int space_remaining;
	int capacity;
	bool is_initialized;
	char * memBlock;
    void * virtual_addr;
} page;
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
	page *owned_pages;	
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

static char * DRAM;
int initblock;

typedef struct pageTable
{
    page ** pages;
    short freePages;
}pageTable;


void * page_alloc(page * curr_page, int numRequested, bool os_mode);

char* findSpace(page * curr_page, int numReq, bool os_mode);
//mergesi contiguous blocks of free memory into a single large block 
void defrag(page * curr_page,bool);

bool page_free (void* target, bool os_mode);

bool segment_free(void * target);

void page_clean();

bool os_free(void *);

void*mallocDetails(int numReq, char * memBlock);

size_t validateInput(page* curr_page, size_t numRequested, bool os_mode);

void initArray(char*);
void * find_page(void *);
pageTable * PT;
int DRAM_INIT;
int PAGE_TABLE_INIT;
int getKey();

int defragPages();
//First find free contigous pages -> if none avaliable use the freePages algorithm and repeat the algorithm, if there is still none and no more pages can be freed -> start choosing victims
page * findFreePages();
//This algorithm goes through the entire block as frees up pages that can be freed!
//If a page is connected to another page an algorithm is put in place to detach the page from the contgious block
int freePages();

int swap(page*, page*);

page * victim();
void * osmalloc(int);
void *mymalloc(size_t);
#endif
