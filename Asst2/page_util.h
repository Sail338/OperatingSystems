#ifndef PAGES_H
#define PAGES_H
#include "thread_util.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define malloc(x) mymalloc (x, __FILE__, __LINE__)
#define free(x) osfree(x, __FILE__, __LINE__)
#define OSLAND 503808

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
    void * virtual_addr;
} page;

typedef struct pageTable
{
    page ** pages;
    short freePages;
}pageTable;

pageTable * PT;
int DRAM_INIT;
int PAGE_TABLE_INIT;

static char * DRAM;
int initblock;

void page_init(page * curr_page);

int getKey();

void * find_page(void *);

void * osmalloc(int);

void *mymalloc(size_t);

void *case_1(int,int);

void* multi_page_alloc(int,int);

int ceil_bytes(int);

page* multi_page_prep(page*,int,int);

void * page_alloc(page * curr_page, int numRequested, bool os_mode);

size_t validateInput(page* curr_page, size_t numRequested, bool os_mode);

char* findSpace(page * curr_page, int numReq, bool os_mode);
//mergesi contiguous blocks of free memory into a single large block 
void defrag(page * curr_page,bool);

page* giveNewPage();

void*mallocDetails(int numReq, char * memBlock);

bool os_free(void *);

bool my_free(void*);

bool page_free (void* target, bool os_mode);

bool segment_free(void * target);

int swap(page*, page*);

void page_clean();
#endif
