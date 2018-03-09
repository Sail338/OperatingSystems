#include "page_alloc.h"
#include <stdlib.h>
#include <stdio.h>

//TODO: make page init function 


//should pass in page struct as param
void* page_alloc (page * curr_page, size_t numRequested, bool os_mode)
{
	//PUT THIS IN THE WRAPPER
		//numRequested = validateInput(curr_page, numRequested);
	char* meta_block;
	//might want to include error message here
	//THIS WILL ALSO GO IN WRAPPER
	//if (numRequested == 0)
	//	return 0;
	if (findSpace(curr_page, numRequested) != NULL)
	{
		meta_block = findSpace(curr_page , numRequested);
	}
	else
	{
		defrag(curr_page);
		meta_block = findSpace(curr_page, numRequested);
	}
	if(meta_block == NULL)
	{
		printf("INSUFFICIENT AVAILABLE MEMORY - ALLOC DENIED \n");
		return 0;
	}
	
	void* test = mallocDetails(numRequested, meta_block);
	return test + 4;//mallocDetails(numRequested, thatSoMeta);	
}

//returns pointer to first incidence of sufficiently large block
char* findSpace(page * curr_page, int numReq)
{
	//tracks how far down the array has been traveled
	int consumed = 0;
	//keeps trace of value contained in current metadata block
	char * currMeta = (char *)(curr_page -> memBlock);
	

	while(consumed < curr_page -> capacity)
	{
		//currMeta = *(int*)(curr_page -> memBlock);
		
		
		//return pointer to start of META (not user!) data block if sufficient size free block is found
		//TOOK AWAY THE PLUS TWO HERE
		if((*(int *)(currMeta)%2==0) && (*(int *)(currMeta)>=numReq))
		{
//			printf("head block: %hu\n", currMeta);
			return (char *)currMeta;
		}
		else
		{
			//catches both free and used jumps through mod arith
			int increment = *currMeta - (*(currMeta)%2) + 4;
			currMeta += increment;
			//increment distanace traveled
			consumed += increment;
		}
	}
	return NULL;
} 

//find contiguous blocks of free space and combine them to a single large block
void defrag (page * curr_page)
{
	int consumed = 0;
	int * home = (int *) (curr_page -> memBlock);
	int * probe = (int *) (curr_page -> memBlock);
	int toAdd = 0;

	//proceed until the end of the memBlock is reached
	while(consumed < curr_page -> capacity)
	{
		//proceed until landing upon first empty block
		while (consumed < curr_page -> capacity  && *home%2 == 1)
		{
			consumed += 4+*home;
			//divide by four because when adding contents of home (i.e. $ of bytes occupied) to home address, will do so by adding them in increments of sizeof(int)
			home += (1+*home)/4;
			//could you also do probe = home?
			probe += (1+*probe)/4;
		}

		probe += (4+*probe)/4;

		//continue hopping along memBlock and consolidating free blocks until you hit a non-free block
		while(consumed<curr_page -> capacity && (*probe)%2==0)
		{
				*home += 4+*probe;
				consumed += 4+*probe;
				//move probe forward by 
				int * temp = probe;
				int forward_jump = *probe;
				*probe = 0;
				probe += (4+forward_jump)/4;
		}
		if (*(probe)%2 == 1)
		{
			home = probe;
		}

	}
}

//return boolean true for success and failure
bool myfree(void* target, page * curr_page)
{
		//not sure why this is necessary, commenting out for now
/*	if (!isInitialized)
		{
			initArray(myBlock);
			isInitialized = TRUE;
		}
	*/
	void* targetFree = target - 4;
	void* ptr = (void*)(curr_page -> memBlock);
	int distance = 0;
	//traverse the memBlock  until you either find the target address or discover that it's invalid
	while (targetFree != ptr && distance < curr_page -> capacity)
	{
		distance += 2+ *(unsigned short*)ptr - (*(unsigned short*)ptr) %2;
		ptr += *(int*)ptr + 4 - (*(int*)ptr)%2;
	}
	//here either targetFree = ptr or distance > curr_page -> capacity
	if (targetFree == ptr)
	{
		//confirm that the memory actually was in use
		if ((*(int*)ptr) %2 == 1)
		{
			*(int*)ptr -= 1;
			return true;
		}
	}
	printf("ERROR: INVALID ADDRESS, CANNOT FREE\n");
	return false;
}

//set initial amount of free space and zero out the array in case of garbage null terminators
void page_init(page * curr_page)
{
	int i = 0;
	int capacity = curr_page -> capacity;
	for (i=0; i < capacity; i++)
	{
		myBlock[i] = '0';
	}
	*(int*)myBlock = curr_page->capacity - 2;
	curr_page -> is_initialized = true;
}

//PUTHIS THIS IN THE WRAPPER FUNCTION
size_t validateInput(page * curr_page, size_t numRequested)
{
	//must be within array bounds
	if (numRequested <= 0 || numRequested > curr_page -> capacity)
		{
			printf("INVALID REQUEST, CANNOT ALLOC\n");
			return 0;
		}
	//allocation must be even
	return (numRequested + numRequested%2);
}

void* mallocDetails(size_t numRequested, char* meta_block)
{
	int  total = *(int*)meta_block;
	if (total > numRequested)
	{
		int* leftovers = (int*) (meta_block+4+numRequested*sizeof(char));
		*leftovers = total - (numRequested+4);
	}

	*(int*)meta_block = numRequested+1;
	//printf("remaining free space: %hu \n", *(unsigned short*)(index+2+numReq*(sizeof(char))));
	return (void*)meta_block;
}
