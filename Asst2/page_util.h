#ifndef PAGES_H
#define PAGES_H
#include "thread_util.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
//#define malloc(x) mymalloc (x, __FILE__, __LINE__)
#define free(x) osfree(x, __FILE__, __LINE__)
#define OSLAND 1048576
#define DRAM_SIZE 8388608
#define SHARED 4 * sysconf(_SC_PAGE_SIZE)
#define SWAP_SIZE 16777215
#define NUM_PAGES_S SWAP_SIZE/sysconf(_SC_PAGE_SIZE)
#define NUM_PAGES  (DRAM_SIZE-OSLAND-SHARED)/sysconf(_SC_PAGE_SIZE)
#define SHALLOC_PAGE NUM_PAGES + NUM_PAGES_S + 1
typedef struct page
{
	//for contigous pages	
	struct page * next_page;
	struct page * prev_page;
	//data type? hello?
	tcb *owner;//tcb == threadNode
	int space_remaining;
	int capacity;
	bool is_initialized;
	char * memBlock;
    char * virtual_addr;
    int fileIndex;
} page;

typedef struct pageTable
{
    page ** pages;
    short free_pages_in_RAM;
    short free_pages_in_swap;
    struct sigaction sa;
	int swapfd;
}pageTable;

pageTable * PT;
int DRAM_INIT;
int PAGE_TABLE_INIT;

static char * DRAM;
int initblock;

void page_init(page * curr_page);

void page_table_initialize(int, int);

int getKey();

void * find_page(void *);

void * osmalloc(int);

void *mymalloc(size_t);

void * single_page_alloc(int,int);

void* multi_page_alloc(int,int);

int ceil_bytes(int);

page* multi_page_prep(page*,int,int);

void * page_alloc(page * curr_page, int numRequested, int mode);

size_t validateInput(page* curr_page, size_t numRequested, int mode);

char* findSpace(page * curr_page, int numReq, int mode);
//mergesi contiguous blocks of free memory into a single large block 
void defrag(page * curr_page,int);

page* giveNewPage();

void*mallocDetails(int numReq, char * memBlock);

bool os_free(void *);

bool my_free(void*);

bool page_free (void* target, int mode);

bool segment_free(void * target);

int swap(page*, page*);

void page_clean();

void page_table_string();

void protect_my_pages();

void unProtect_my_pages(threadNode*);

page* page_defrag(page*,int,int);

page * find_page_virtual_addr(void*);

static void page_fault_handler(int,siginfo_t *, void*);

void unprotectAll();

void * evict(int);

void protectAll();

int randNum(int,int);

void createSwap();

void moveToSwap(page *);

bool cheack_possible_free(int);

void unprotectFree();

void * shalloc(size_t);
#endif
