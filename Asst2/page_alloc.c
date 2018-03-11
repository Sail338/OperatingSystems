#include "page_alloc.h"
page * findFreePages()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    void * ptr = myBlock + OSLAND;
    //Page has all of its space 
    if((page*)ptr->capacity == pageSize)
    {
          
    }
}

int getKey(void * virtualAddr)
{
    void * ptr = myBlock + OSLAND;
    int pageSize = sysconf(_SC_PAGE_SIZE);
    int i = 0;
    int numOfPages = (8000000000 - OSLAND) / pageSize;
    while(i < numOfPages){
        if ptr == virtualAddr:
            return i;
        ptr += pageSize;
        i += 1;
    }
    return -1;
}

int swap(page * p1, page * p2)
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    char temp[pageSize];
    memcpy(temp,p1->physical_addr,pageSize);
    memcpy(p1->physical_addr,p2->physical_addr,pageSize);
    memcpy(p2->physical_addr,temp,pageSize);
    void * t = p1->physical_addr;
    p1->physical_addr = p2->physical_addr;
    p2->physical_addr = t;
    int p1Info = getKey(p1->virtual_addr);
    int p2Info = getKey(p2->virtual_addr);
    page * tempPtr = pageTable->pages+p1Info;
    pageTable->pages+p1Info = pageTable->pages+p2Info;
    pageTable->pages+p2Info = tempPtr;
    
}

int initialize()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    myBlock  = memalign(pageSize,8388608);
    int i = 0;
    for(int i = 0; i < 8388608; i++)
    {
        myBlock[i] = 0;
    }
    int numOfPages = (8388608-OSLAND)/pageSize;
    initblock = 1;
    PT = osmalloc(myBlock,sizeof(pageTable));
    PT->freePages = numOfPages;
    PT->pages = osmalloc(myBlock,sizeof(page*)*numOfPages);
    int i;
    void * ptr = myBlock + OSLAND;
    for(i = 0; i < numOfPages; i++){
        PT->pages[i] = osmalloc(myBlock,sizeof(page));
        PT->pages[i]->physical_addr = ptr;
        PT->pages[i]->virtual_addr = ptr;
        PT->pages[i]->next = NULL;
        PT->pages[i]->prev = NULL;
        PT->pages[i]->owner = NULL;
        PT->pages[i]->space_remaining = pageSize;
        PT->pages[i]->capacity = pageSize;
        PT->pages[i]->is_initialized= 0;
        i+=1;
        ptr += pageSize;
    }
    if(init == 0)
    {
        initScheduler();
    }
}



//TODO: make page init function 


//should pass in page struct as param
void* page_alloc (page * curr_page, size_t numRequested, bool os_mode)
{
	//PUT THIS IN THE WRAPPER
	//numRequested = validateInput(curr_page, numRequested);
	char* thatSoMeta;
	//might want to include error message here
	//THIS WILL ALSO GO IN WRAPPER
	//if (numRequested == 0)
	//	return 0;
	if (checkSpace(myBlock, numRequested))
	{
		thatSoMeta = findSpace(myBlock, numRequested);
	}
	else
	{
		defrag(myBlock);
		printf("defragged\n");
		thatSoMeta = findSpace(myBlock, numRequested);
	}
	if(thatSoMeta == NULL)
	{
		printf("INSUFFICIENT AVAILABLE MEMORY - ALLOC DENIED \n");
		return 0;
	}
	
	void* test = mallocDetails(numRequested, thatSoMeta);
	//printf("num allocated: %hu \n", *(short*)(test));
	return test + 2;//mallocDetails(numRequested, thatSoMeta);	
}

//returns pointer to first incidence of sufficiently large block
char* findSpace(page * curr_page, int numReq)
{
	//tracks how far down the array has been traveled
	int consumed = 0;
	//keeps trace of value contained in current metadata block
	int * currMeta = curr_page -> memBlock;
	
	while(consumed < curr_page -> capacity)
	{
		*currMeta = *(int *)curr_page -> memBlock;
		//return pointer to start of META (not user!) data block if sufficient size free block is found
		//TOOK AWAY THE PLUS TWO HERE
		if((*(currMeta)%2==0) && (*(currMeta)>=numReq))
		{
//			printf("head block: %hu\n", currMeta);
			return myBlock;
		}
		else
		{
			//catches both free and used jumps through mod arith
			unsigned short increment = *currMeta - (*(currMeta)%2) + 2;
//			printf("currMeta: %hu \t increment: %hu \n", currMeta, increment);
			myBlock += increment*sizeof(char);
			//increment distanace traveled
			consumed += increment;
			//printf("consumed: %hu \n", consumed);
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
			consumed += 2+*home;
			//divide by four because when adding contents of home (i.e. $ of bytes occupied) to home address, will do so by adding them in increments of sizeof(int)
			home += (1+*home)/4;
			//could you also do probe = home?
			probe += (1+*probe)/4;
		}

		probe += (2+*probe)/4;

		while(consumed<curr_page -> capacity && (*probe)%2==0)
		{
				*home += 2+*probe;
				consumed += 2+*probe;
				probe += (2+*probe)/2;
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
	void* ptr = (void*)myBlock;
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
		myBlock[i] = '0';
	}
	*(int*)myBlock = curr_page->capacity - 2;
	curr_page -> is_initialized = true;
}

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

void* mallocDetails(size_t numReq, char* index)
{
	unsigned short total = *(unsigned short*)index;
	if (total > numReq)
	{
		unsigned short* leftovers = (unsigned short*) (index+2+numReq*sizeof(char));
		*leftovers = total - (numReq+2);
	}

	*(unsigned short*)index = numReq+1;
	//printf("remaining free space: %hu \n", *(unsigned short*)(index+2+numReq*(sizeof(char))));
	return (void*)index;
}
