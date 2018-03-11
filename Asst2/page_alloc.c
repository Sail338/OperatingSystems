#include "util.h"
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
	return 0;
    
}
/**
 *
 *
 * Initializes DRAM , gives us 8 MB with melaign and then 0s out OSSPACE
 * and then sets the metadata in OSSPACE to the appropriate amount
 *
 *
 * */
int initialize()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    DRAM  = (char *)memalign(pageSize,8388608);
    int i;
    for(i = 0; i < OSLAND; i++)
    {
        DRAM[i] = 0;
    }
	*(int *)DRAM = OSLAND-4;
	return 0;
}


/**
 *@param curr_page a empty page or a page a thread owns with enough space
 @param numRequested number of blocks to be allocated
 *@param os_mode an OSMODE flag which tells page_alloc to allocate in osland,curr_page should be null and is ignored if this flag is set to true

 *page_alloc takes in a pointer to a page and allocates the appropriate amount
 *assumes that the block given is also contigous so page_alloc does NOT do any page swapping
 *
 * **/

//should pass in page struct as param
void* page_alloc (page * curr_page, int numRequested, bool os_mode)
{
	numRequested = validateInput(curr_page, numRequested, os_mode);
	//PUT THIS IN THE WRAPPER
	//numRequested = validateInput(curr_page, numRequested);
	char* thatSoMeta = findSpace(curr_page, numRequested,os_mode);
		//numRequested = validateInput(curr_page, numRequested);
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
	
	if(os_mode){
				*(int *) currMeta = *(int *)DRAM;
		}
		else{
				*(int *)currMeta = *(int *)curr_page -> memBlock;
	

		}
		int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND-4;
		
		while(consumed < maxSize)
		{
			//return pointer to start of META (not user!) data block if sufficient size free block is found
			if(((*(int *)currMeta)%2==0) && (*(int *)currMeta)>=numReq)
			{
				return currMeta;
			}
			else
			{
				//catches both free and used jumps through mod arith
				int sub = *(int *)currMeta;
				int increment = sub - (sub%2) + 4;
				//update currMeta to point to next metadata block		
				currMeta += increment*sizeof(char);
				//increment distanace traveled
				consumed += increment;
			}
			//catches both free and used jumps through mod arith
		}
	
	return NULL;
} 

//find contiguous blocks of free space and combine them to a single large block

//THIS IS DEFINITELY FULL OF BUGS WITH POINTER INCREMENTATION
void defrag (page * curr_page,bool os_mode)
{
		//currently we dont defrag in os_mode
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

		//continue hopping along memBlock and consolidating free blocks until you hit a non-free block
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
/**
 *@param target  target pointer
 @param os_mode flag if we are in os_mode or not
 the main function our wrapper function will call to free a block,
 free() calls os_mode = false with a page and if within a page, we calcualte the apporpate page
 os_free is for the OS ONLY and it works like system free expect with a bigger chunk
 *
 *
 **/
bool page_free(void * target, bool os_mode)
{
	//FIRST THING YOU DO IF OS MODE IS FALSE IS CALL THE HELPER FUNCTION TO FIND THE PAGE
	//calc boundaries	
	void * end;
	if (os_mode == true)
	{
		end = (void *)(DRAM + OSLAND);
	}
	void* targetMeta = target - 4;

	//check that the section you're trying to free is within os space/user page bounds
	//THIS CODE SEGMENT IS JUST FOR THE OS RIGHT NOW
	if (targetMeta >= (void *)DRAM && targetMeta < (void *)(DRAM+OSLAND-5))
	{
		if (segment_free(targetMeta) == true)
			return true;
	}
	
	printf ("INVALID ADDRESS, CANNOT FREE\n");
	return false;
}
/**
 *Helper function that page_free uses to free a block
 *checks if metadata is ine use and decrements one so metadata is even and is usable now
 *
 * */
bool segment_free(void * target)
{
	if (*(int *) target %2 == 1)
	{
		*(int *) target -= 1;
		return true;
	}
	printf("SPACE NOT IN USE\n");
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
//PUTHIS THIS IN THE WRAPPER FUNCTION
{
	int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND-4;
	//must be within array bounds
	if (numRequested <= 0 || numRequested > maxSize)
		{
			printf("INVALID REQUEST, CANNOT ALLOC\n");
			return 0;
		}
	numRequested += numRequested %2;
	//allocation must be even
	return numRequested;
}

/**
 *@param numReq number of bytes passed requested
 @param the block of memory operating os (for OSmod its osland)
 *Helper function to update metadatas
 *
 * */
void* mallocDetails(int numReq, char* memBlock)
{
	int total = *( int*)memBlock;
	if (total > numReq)
	{
		char * leftovers = memBlock+4+numReq;
		*(int *)leftovers = total-(numReq+4);
	}
	*(int*)memBlock = numReq+1;
	//printf("remaining free space: %hu \n", *(unsigned short*)(index+2+numReq*(sizeof(char))));
	return (void*)(memBlock+4*sizeof(char));
}

/**
 *Function used by scheduler, and our memory manager ONLY to allocate in 
 os land, the user cant call/shouldnt call this
 *
 *
 * **/
void* osmalloc(int bytes)
{

	//inits DRAM if it hasnt already be init, an
	if(DRAM_INIT ==0)

	{
		
			initialize();
			DRAM_INIT =1;
	}
	void  *x =page_alloc(NULL,bytes,true);	
	if(x >= (void *)(DRAM + OSLAND))
	{
		return NULL;
	}
	return x;
}

//TODO: WRITE A FIND_PAGE HELPER FUNCTION FOR USER FREE TO FIND THE PAGE WITHIN WHICH THEY'RE TRYING TO FREE

//JUST CALLS PAGE_FREE FOR OS 
bool os_free(void * target)
{
	return page_free(target, true);
}
/**
 *@param number of bytes we want
 * Our actual malloc function (user calls this one)
 *
 *
 * */
void *mymalloc(size_t bytes){
	/**
	 *
	 *The function that the user is going to call
	 * 
	 * **/
	//inits page table if it alreadt hasnt been
	bytes = 0;
	if(!PAGE_TABLE_INIT){
		//initialize page table and then scheduler
		
    	int pageSize = sysconf(_SC_PAGE_SIZE);
    	int numOfPages = (8388608-OSLAND)/pageSize;

    	//allocate space for the pageTable struct 	
		 PT = (pageTable *)osmalloc(sizeof(pageTable));

    	  PT->freePages = numOfPages;
    	  PT->pages = osmalloc(sizeof(page*)*numOfPages);
   		 void * ptr = (void*)(DRAM + OSLAND);
		 int i;
   		 for(i = 0; i < numOfPages; i++){
        	PT->pages[i] = osmalloc(sizeof(page));
       		PT->pages[i]->memBlock = ptr;
        	PT->pages[i]->virtual_addr = ptr;
        	PT->pages[i]->next_page = NULL;
        	PT->pages[i]->prev_page = NULL;
       		PT->pages[i]->thread = NULL;
       		PT->pages[i]->space_remaining = pageSize;
        	PT->pages[i]->capacity = pageSize;
        	PT->pages[i]->is_initialized= false;
        	ptr += pageSize;
   	 	}
		 //if shceduler has not be initalized call initScheduler()
    	if(init == 0)
    	{
        	initScheduler();
    	}
		PAGE_TABLE_INIT =1;

	}
	return NULL;

}
