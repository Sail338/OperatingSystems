#include "page_alloc.h"
page * findFreePages()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    void * ptr = DRAM + OSLAND;
    //Page has all of its space 
    if(((page*)ptr)->capacity == pageSize)
    {
          
    }
}

int getKey(void * virtualAddr)
{
    void * ptr = DRAM + OSLAND;
    int pageSize = sysconf(_SC_PAGE_SIZE);
    int i = 0;
    int numOfPages = (8000000000 - OSLAND) / pageSize;
    while(i < numOfPages){
        if (ptr == virtualAddr){
            return i;
        ptr += pageSize;
        i += 1;
			}
    }
    return -1;
}

int swap(page * p1, page * p2)
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    char temp[pageSize];
    memcpy(temp,p1->memBlock,pageSize);
    memcpy(p1->memBlock,p2->memBlock,pageSize);
    memcpy(p2->memBlock,temp,pageSize);
    void * t = p1->memBlock;
    p1->memBlock = p2->memBlock;
    p2->memBlock = t;
    int p1Info = getKey(p1->virtual_addr);
    int p2Info = getKey(p2->virtual_addr);
    page * tempPtr = PT->pages[p1Info];
    PT->pages[p1Info] = PT->pages[p2Info];
    PT->pages[p2Info] = tempPtr;
    
}

int initialize()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    DRAM  = memalign(pageSize,8388608);
    int i;
    for(i = 0; i < OSLAND; i++)
    {
        DRAM[i] = 0;
    }
	*(int *)DRAM = OSLAND-4;
    int numOfPages = (8388608-OSLAND)/pageSize;
    initblock = 1;

    	
		 /* PT = (pageTable *)osmalloc(myBlock,sizeof(pageTable));
    PT->freePages = numOfPages;
    PT->pages = osmalloc(myBlock,sizeof(page*)*numOfPages);
    void * ptr = myBlock + OSLAND;
    for(i = 0; i < numOfPages; i++){
        PT->pages[i] = osmalloc(myBlock,sizeof(page));
        PT->pages[i]->memBlock = ptr;
        PT->pages[i]->virtual_addr = ptr;
        PT->pages[i]->next_page = NULL;
        PT->pages[i]->prev_page = NULL;
        PT->pages[i]->thread = NULL;
        PT->pages[i]->space_remaining = pageSize;
        PT->pages[i]->capacity = pageSize;
        PT->pages[i]->is_initialized= 0;
        i+=1;
        ptr += pageSize;
    }*/
    if(init == 0)
    {
        //initScheduler();
    }
}



//TODO: make page init function 


//should pass in page struct as param
void* page_alloc (page * curr_page, int numRequested, bool os_mode)
{
	if(DRAM_INIT ==0){
			initialize();
			DRAM_INIT =1;
		}
	//PUT THIS IN THE WRAPPER
	//numRequested = validateInput(curr_page, numRequested);
	char* thatSoMeta = findSpace(curr_page, numRequested,os_mode);
	//might want to include error message here
	//THIS WILL ALSO GO IN WRAPPER
	//if (numRequested == 0)
	//	return 0;
	if (thatSoMeta == NULL)
	{
		defrag(curr_page,os_mode);
		printf("defragged\n");
		thatSoMeta = findSpace(curr_page, numRequested, os_mode);
	}
	if(thatSoMeta == NULL)
	{
		printf("INSUFFICIENT AVAILABLE MEMORY - ALLOC DENIED \n");
		return 0;
	}
	
	void* usable_space = mallocDetails(numRequested, thatSoMeta);
	//printf("num allocated: %hu \n", *(short*)(test));
	return usable_space;//mallocDetails(numRequested, thatSoMeta);	
}

//returns pointer to METADATA BLOCK of first incidence of sufficiently large block
char* findSpace(page * curr_page, int numReq,bool os_mode)
{
	//tracks how far down the array has been traveled
	int consumed = 0;
	//keeps trace of value contained in current metadata block
	void * currMeta;
	if(os_mode){
		currMeta = DRAM;
	}	
	else{
		 currMeta = curr_page -> memBlock;
	}
	
	int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND;
		
		while(consumed < maxSize)
		{
			if(os_mode){
				*(int *) currMeta = *(int *)DRAM;
			}
			else{
				*(int *)currMeta = *(int *)curr_page -> memBlock;
			}
			//return pointer to start of META (not user!) data block if sufficient size free block is found
			if(((*(int *)currMeta)%2==0) && (*(int *)currMeta)>=numReq)
			{
				return currMeta;
			}
			else
			{
				//catches both free and used jumps through mod arith
				int increment = *(int *)currMeta - *(int *)currMeta%2 + 1;
				//update currMeta to point to next metadata block		
				currMeta += increment*sizeof(char);
				//increment distanace traveled
				consumed += increment;
			}
		}
	
	return NULL;
} 

//find contiguous blocks of free space and combine them to a single large block

//THIS IS DEFINITELY FULL OF BUGS WITH POINTER INCREMENTATION
void defrag (page * curr_page,bool os_mode)
{
	if(os_mode){
			return;
		}
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
			//update consumed by amount of space after metadata block, plus size of metadata block itself
			consumed += 4+*home;

			//divide by four because when adding contents of home (i.e. $ of bytes occupied) to home address, will do so by adding them in increments of sizeof(int)
			home += 1+*home;
			probe += 1+*probe;
		}

		probe += 1+*probe;

		while(consumed<curr_page -> capacity && (*probe)%2==0)
		{
				*home += 1+*probe;
				consumed += 1+*probe;
				probe += 1+*probe;
		}
		if (*(probe)%2 == 1)
		{
			home = probe;
		}

	}
}

//return boolean true for success and failure
bool myfree(void* target, char* file, int line)
{
//	printf("in target: %p \n", (void*)target);
//	printf ("target: %hu \n", *(unsigned short*)(target-2));
	void* targetFree = target - 2;
	void* ptr = (void*)DRAM;
	unsigned short distance = 0;
	while (targetFree != ptr && distance < 5000)
	{
		distance += 2+ *(unsigned short*)ptr - (*(unsigned short*)ptr) %2;
//		printf("DISTANCE: %hu \n", distance);
		ptr += *(unsigned short*)ptr +2 - (*(unsigned short*)ptr)%2;
	}
	//here either targetFree = ptr or distance > 5000
	if (targetFree == ptr)
	{
	//	printf("%hu \n", *(unsigned short*)ptr);
		if ((*(unsigned short*)ptr) %2 == 1)
		{
//			printf("merp\n");
			*(unsigned short*)ptr -= 1;
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
		curr_page->memBlock[i] = '0';
	}
	*(int*)curr_page->memBlock = curr_page->capacity - 4;
	curr_page -> is_initialized = true;
}

size_t validateInput(page * curr_page, size_t numRequested,bool os_mode)
{
	int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND;
	//must be within array bounds
	if (numRequested <= 0 || numRequested >= maxSize)
		{
			printf("INVALID REQUEST, CANNOT ALLOC\n");
			return 0;
		}
	//allocation must be even
	return (numRequested + numRequested%2);
}

void* mallocDetails(int numReq, char* memBlock)
{
	int total = *( int*)memBlock;
	if (total > numReq)
	{
		void * leftovers = (int *)(memBlock)+1+numReq;
		*(int *)leftovers = total-(numReq+4);
	}
	*(int*)memBlock = numReq+1;
	//printf("remaining free space: %hu \n", *(unsigned short*)(index+2+numReq*(sizeof(char))));
	return (void*)(memBlock+4*sizeof(char));
}

void* osmalloc(int bytes){
	void  *x =page_alloc(NULL,bytes,true);	
	if(x == DRAM + OSLAND){
		return NULL;
	}
	return x;
}
