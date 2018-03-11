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
	void * physical_addr;
	int space_remaining;
	int capacity;
	boolean is_initialized;
	char * memBlock;
    void * virtual_addr;
} page;

typedef struct pageTable
{
    page ** pages;
    short freePages;
}pageTable;


//returns pointer to block of memory if sufficient space remains in myBlock
void* mymalloc(size_t numRequested, char* file, int line);

bool checkSpace(char* myBlock, size_t numReq);

char* findSpace(char* myBlock, unsigned short numReq);
//merges contiguous blocks of free memory into a single large block 
void defrag(char* myBlock);

bool myfree (void* p, char* file, int line);

void* mallocDetails(size_t numRequested, char* index);

size_t validateInput(page* curr_page, size_t numRequested);

void initArray(char*);

pageTable * PT;

int getKey();

int defragPages();

page * findFreePages();

int freePages();

int swap(page*, page*);
