#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#define malloc(x) mymalloc (x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)


//malloc array - holds metadata as a short and spaces allocated for user data
static char myBlock [5000];
typedef struct page
{
	threadNode * thread;
	struct page * next_page;
	struct page * prev_page;
	//data type? hello?
	void * virtual_addr;
	int space_remaining;
	int capacity;
	bool is_initialized;
	void * memBlock;
} page;

//returns pointer to block of memory if sufficient space remains in myBlock
void* mymalloc(size_t numRequested, char* file, int line);

char* findSpace(page * curr_page, int numReq);
//merges contiguous blocks of free memory into a single large block 
void defrag(page * curr_page);

bool myfree (void* target, page* curr_page);

void* mallocDetails(size_t numRequested, char* meta_block);

size_t validateInput(page* curr_page, size_t numRequested);

//THIS SHOULD BE A WRAPPER FUNCTION, *NOT* A PAGE FUNCTION
void initArray(char* myBlock);
