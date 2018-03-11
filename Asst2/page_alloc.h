#include <malloc.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include "util.h"
#define malloc(x) mymalloc (x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)
#define OSLAND 5000000

//malloc array - holds metadata as a short and spaces allocated for user data
//Need to memalign myBlock so do this in init
static char * myBlock;
int initblock;
typedef struct page
{
	threadNode * thread;
	struct page * next_page;
	struct page * prev_page;
	//data type? hello?
	int space_remaining;
	int capacity;
	bool is_initialized;
	void * memBlock;
    void * virtual_addr;
} page;

typedef struct pageTable
{
    page ** pages;
    short freePages;
}pageTable;


void * page_alloc(page * curr_page, int numRequested, bool os_mode);

char* findSpace(page * curr_page, int numReq);
//mergesi contiguous blocks of free memory into a single large block 
void defrag(page * curr_page);

bool myfree (void* p, char* file, int line);

void* mallocDetails(size_t numRequested, char* index);

size_t validateInput(page* curr_page, size_t numRequested);

void initArray(char*);

pageTable * PT;

int getKey();

int defragPages();
//First find free contigous pages -> if none avaliable use the freePages algorithm and repeat the algorithm, if there is still none and no more pages can be freed -> start choosing victims
page * findFreePages();
//This algorithm goes through the entire block as frees up pages that can be freed!
//If a page is connected to another page an algorithm is put in place to detach the page from the contgious block
int freePages();

int swap(page*, page*);

page * victim();
