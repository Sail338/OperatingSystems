#include "page_util.h"



static void page_fault_handler(int sig, siginfo_t *si, void *unsued)
{
   printf("In the Page Fault Signal Handler!\n");
   unprotectAll();
   page * real_page = find_page_virtual_addr(si->si_addr);
   page * fake_page = find_page(si->si_addr);
   if(real_page == NULL || fake_page == NULL)
   {
    return;
   }
   int i;
   void * curr = fake_page->memBlock;
   if(real_page->prev_page == NULL && real_page->next_page == NULL)
   {
    swap(real_page,fake_page);
   }
   while(real_page->prev_page != NULL)
   {
    real_page = real_page->prev_page;
    curr -= sysconf(_SC_PAGE_SIZE);
   }
   while(real_page->next_page != NULL)
   {
        page * swapped = find_page(curr);
        swap(real_page,swapped);
        real_page = real_page->next_page;
        curr += sysconf(_SC_PAGE_SIZE);
   }
   protectAll();
   printf("Finished Page Fault Hanlder!\n");

}

void unprotectAll()
{
   mprotect(DRAM + OSLAND,NUM_PAGES*sysconf(_SC_PAGE_SIZE),PROT_READ | PROT_WRITE);
}

void protectAll()
{
    int i;
	for(i=0;i<NUM_PAGES;i++)
    {
	    if(scheduler ->current != PT->pages[i]->owner)
        {
			if(PT->pages[i]->owner == NULL)
            {
				continue;
			}	
            printf("Virutal Addr Protect: %p\n",PT->pages[i]->virtual_addr);
			mprotect(PT->pages[i]->memBlock,sysconf(_SC_PAGE_SIZE),PROT_NONE);
		}

	}
}


page * find_page_virtual_addr(void * target)
{
    int i;
    for(i = 0; i < NUM_PAGES; i++)
    {
        if(PT->pages[i]->owner == scheduler->current 
                && (char*)target >= PT->pages[i]->virtual_addr
                && (char*)target <= PT->pages[i]->virtual_addr+sysconf(_SC_PAGE_SIZE))
        {
            return PT->pages[i];
        }
    }
    return NULL;
}

/**
 * Initializes DRAM , gives us 8 MB with memalign and then 0s out OSSPACE
 * and then sets the metadata in OSSPACE to the appropriate amount
 * */
int DRAM_initialize()
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    DRAM  = (char *)memalign(pageSize,DRAM_SIZE);
    int i;
    for(i = 0; i < OSLAND; i++)
    {
        DRAM[i] = 0;
    }
	*(int *)DRAM = OSLAND-4;
	return 0;
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
/**
 *Hash function to get the index in the pagetable given a pageadress
 *ie maps 0x80000 -> 1 
 * */


//inits page table if it hasn't already been
void page_table_initialize(int pageSize, int numOfPages)
{
	//initialize page table and then scheduler
    //allocate space for the pageTable struct 	
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = page_fault_handler;
	PT = (pageTable *)osmalloc(sizeof(pageTable));
   	PT->freePages = numOfPages;
    PT->pages = osmalloc(sizeof(page*)*numOfPages);
    void * ptr = (void*)(DRAM + OSLAND);
	int i;
	//initializing the pages in page table with default values
   	for(i = 0; i < numOfPages; i++)
	{
    	PT->pages[i] = osmalloc(sizeof(page));
    	PT->pages[i]->memBlock = ptr;
        PT->pages[i]->virtual_addr = ptr;
        PT->pages[i]->next_page = NULL;
        PT->pages[i]->prev_page = NULL;
		PT->pages[i]->owner = NULL;
       	PT->pages[i]->space_remaining = pageSize;
        PT->pages[i]->capacity = pageSize;
        PT->pages[i]->is_initialized= false;
        ptr += pageSize;
   	 }
	//if scheduler has not be initalized call initScheduler()
    if(init == 0)
    {
        initScheduler();
    }
	PAGE_TABLE_INIT =1;
}

int getKey(void * virtualAddr)
{
    void * ptr = DRAM + OSLAND;
    int pageSize = sysconf(_SC_PAGE_SIZE);
    int i = 0;
    int numOfPages = (DRAM_SIZE - OSLAND) / pageSize;
    while(i < numOfPages){
        if (ptr == virtualAddr)
		{
            return i;
		}
        
        ptr += pageSize;
        i += 1;
    }
    return -1;
}

//this is used in user free functions; it finds the page which contains the pointer they're trying to free
/**
 *@param target : given a target find the page the target is in
 	This function maps a target to its relevant page, is useful in free when we free a pointer inside a page so we can update page struct
 *
 * **/
void* find_page(void * target)
{
	void * index = (void *)(DRAM + OSLAND);
	int pageSize = sysconf(_SC_PAGE_SIZE);
    int pageNum = 0;
    int numPages = (DRAM_SIZE - OSLAND) / pageSize;
	for (pageNum=0; pageNum<numPages; pageNum++)
	{
		if (target >= index && target < index + pageSize)
		{
			//printf("The page number is %d\n",pageNum);
			return (void *)(PT->pages[pageNum]);
		}
		index += pageSize;
	}
	return NULL;
}


/**************** MALLOCY THINGS ***************/

/**
 *Function used by scheduler, and our memory manager ONLY to allocate in 
 os land, the user cant call/shouldnt call this
 * **/
void* osmalloc(int bytes)
{

	//inits DRAM if it hasnt already be init, an
	if(DRAM_INIT ==0)

	{
		
			DRAM_initialize();
			DRAM_INIT =1;
	}
	void  *x =malloc(bytes);	
	/*if(x >= (void *)(DRAM + OSLAND))
	{
		return NULL;
	}*/
	return x;
}

/**
 *@param numRequested : number of bytes the user wants
 *This is the actual malloc function the user calls
 * */
void *mymalloc(size_t numRequested)
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    int numPages = (DRAM_SIZE-OSLAND)/pageSize;
	if (!PAGE_TABLE_INIT)
	{
		page_table_initialize(pageSize, numPages);
	}

	__atomic_store_n(&(scheduler->SYS),true,__ATOMIC_SEQ_CST);	
	//CASE 1 if the bytes allocated is less than the a page, check owned pages to see your page has enough space
	if((int)numRequested <= (sysconf(_SC_PAGE_SIZE)-4)){
		//grab the current context
		
		void* to_ret =  single_page_alloc(numRequested,numPages);
		__atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
		return to_ret;
	}

	//case 2 MORE THAN A PAGE
	else
	{
			
		void* to_ret = multi_page_alloc	(numRequested,numPages);	
		__atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
		return to_ret;
		//if multi page alloc returns null, then move pages around and if that fails, swap with disk
	}
		//swap files
	
	return NULL;
}
/**
 *@param numRequested : bytes requested
 @param numPages numOfPages total in the pagetable
 *this is the helper function for case 1 less than a page
 * */
void * single_page_alloc(int numRequested,int numOfPages)
{

		threadNode * curr = scheduler->current;
		int i =0;
		//iterate through owned pages and check if any of them having the appropriate space left
		for(i=0;i<numOfPages;i++){
			page* ptr = PT->pages[i];
			if(ptr->owner == curr){		
				if(ptr->space_remaining >= ((int)numRequested)+4){
					while(ptr->prev_page != NULL)
					{
						ptr = ptr -> prev_page;
					}
					void *first_try = page_alloc(ptr,numRequested,false);
					if(first_try != NULL){
						return first_try;
				}
			}
		}
	}
		//give a new page if we have to
		void * second_try = giveNewPage();
		//with the new page make call to page_alloc this SHOULD RETURN TRUE 
		void *to_ret = page_alloc(second_try,numRequested,false);
		if(to_ret != NULL){
			return to_ret;
		}

}
/**
 *This is function is called in the second case where user requests MORE Than a page -4
 *@param numRequested: num bytes the USER requests
 * @param numOfPages number of pages in the page table
 *
 *
 *
 * */
void * multi_page_alloc(int numRequested,int numOfPages)

{
		 //calculate number of pages needed
		int num_pages_needed = ceil_bytes(numRequested); 
		//check if there are enough free pages, if not then grab from swap
		if(PT->freePages >= num_pages_needed){
		//check if there are n contgious pages in DRAM 	
			int i;
			int contig = 0;
			int max = -1;
			page *max_contig = NULL;
			page *start_contig = NULL;
			
			for(i=0;i<numOfPages;i++){
		
				page* pg = find_page(DRAM+OSLAND + sysconf(_SC_PAGE_SIZE) * i);			
				if(pg ->is_initialized == false){
					if(contig ==0){
						start_contig = pg;
					}
					contig ++;
				
				}
				if(contig == num_pages_needed){
					void * to_alloc = multi_page_prep(start_contig,num_pages_needed,numRequested);;
					PT->freePages -= num_pages_needed;
					return page_alloc(to_alloc,numRequested,false);
				}
				else{
					if(pg->is_initialized == true){
						if(contig >max){
							max = contig;
							max_contig = start_contig;
						}
						contig = 0;
						start_contig = NULL;
					}
				}

			}			
			//if for loop is done and we cannot find anything then pass in max_contig
			page *ret = page_defrag(max_contig,max,num_pages_needed);
		//	page * ret = NULL;
			if(ret == NULL){
				//swap from swap file
				return NULL;

			}
			else{
					void * to_alloc = multi_page_prep(ret,num_pages_needed,numRequested);;
					PT->freePages -= num_pages_needed;
					return page_alloc(to_alloc,numRequested,false);
			}
			
		}

		return NULL;
}

//given a pointer to largest contig block try to swap pages untill the numeber u need
/**
 *@param currentLargest: pointer to largest chunk multipage could find
 *@param sizeCurrentLargest: number of pages in largest chunk
 *@param numNeeded: number of pages needed in TOTAL
 * */
page  *page_defrag(page *currentLargest,int sizeCurrentLargest,int numNeeded)

{
		//first enter sys mode
		//unprotect all the pages so we can page swap
        unprotectAll();
		//current pointer to the endBlock (last free page)
		page *end_page = find_page(currentLargest->memBlock + (sizeCurrentLargest-1)*sysconf(_SC_PAGE_SIZE));	
		//END OF DRAM
		char *end = DRAM + OSLAND + (NUM_PAGES *(sysconf(_SC_PAGE_SIZE)));
		//number of pages we have currently
		int page_current = sizeCurrentLargest;
		//intialize current to NULL
		char *curr = NULL;
		//intialize what we are going to return to NULL
		page* to_ret = NULL;
		//check if 2 after the last page of free block is outside of out threashold
		if(end_page->memBlock + 2*sysconf(_SC_PAGE_SIZE) < end){
			 curr = end_page->memBlock + 2*sysconf(_SC_PAGE_SIZE);
		}
		//expand to end
		//while we dont have enough enoughpages and we are within DRAM
		//part one EXPAND TO THE END
		if(curr != NULL){
			while(page_current < numNeeded && curr < end){
				//find the page that ihe current memBlock is pointing to
				page *check_if_free = find_page(curr);
				//if that page is free swap with the end and incremnt what we have by one
				if(check_if_free ->is_initialized == false && check_if_free->owner != scheduler ->current){
			 		end_page = find_page(end_page->memBlock + sysconf(_SC_PAGE_SIZE));
				 	swap(end_page,check_if_free);
				 	page_current +=1; 
				}
			//move curr equal to the next page to inspect
			curr += sysconf(_SC_PAGE_SIZE);
		

		}
	}
	//after expanding to the end and if we found enough pages set what we want to return to currentLargest which is the start of the block
	if(page_current == numNeeded){
		to_ret = currentLargest;	
	}
	//PART two: if we couldnt find enough pages to swap, expand from the start backwards	
	else{
		curr = currentLargest ->memBlock - 2*(sysconf(_SC_PAGE_SIZE));
		while(page_current < numNeeded && curr > DRAM+OSLAND){
			//find the page that ihe current memBlock is pointing to
			page *check_if_free = find_page(curr);
			//if that page is free swap with the end and incremnt what we have by one
			if(check_if_free ->is_initialized == false && check_if_free->owner != scheduler->current){
				 currentLargest = find_page(end_page->memBlock - sysconf(_SC_PAGE_SIZE));
				 swap(currentLargest,check_if_free);
			 	 page_current +=1; 
			}
		//move curr equal to the next page to inspect
			curr -= sysconf(_SC_PAGE_SIZE);
		}
	}
	//now if we found enough set to_ret = to the front
	if(page_current == numNeeded){
		to_ret = currentLargest;

	}
	//protect the pages again and set sysmode back to false
    protectAll();
	return to_ret;

}

/**
 *@param int numBytes: Number of bytes the user requests
 *
 *Takes the Ceiling of the number of pages needed better to overestimate than underestimate :>)
 *
 * */
int ceil_bytes(int numBytes)
{
	double pg_size = (double)sysconf(_SC_PAGE_SIZE);
	//add 4 to account for metadata and weird overflow
	double bytes =  (double)numBytes+4;
	double divided = bytes/pg_size;
	int x = (int)divided;
	if (divided - (double)x> 0){
		return x+1;
	}
	return x;
}
/**
 * This Function prepares a multi page alloc , basically by building a linked list and setting relvant data 
 *
 *@param start : start of a change
 *@param int: num_pages_needed : Number of pages to chain
 *@param int: numRequested: Num Bytes the USER requested
 *
 *
 * */
page *multi_page_prep(page *start,int num_pages_needed,int numRequested)
{
	//build the linked list and set metadata and set other pages is_init = true
	start ->capacity = num_pages_needed * sysconf(_SC_PAGE_SIZE);
	start -> space_remaining = start ->capacity;
	start -> owner = scheduler -> current;
	start->is_initialized = true;
	//set intial adress = memBlock so we can swap back, because usr will a;ways acess va
	start->virtual_addr = start->memBlock;
	page * ptr = start;
	page_init(ptr);	
	int i;
	start ->prev_page = NULL;
	for(i=0;i<num_pages_needed;i++)
	{
		if(ptr == start){
			start -> next_page = find_page(DRAM + OSLAND + sysconf(_SC_PAGE_SIZE)*(i+1));	

		}
		else{
			ptr ->prev_page = find_page(DRAM + OSLAND + sysconf(_SC_PAGE_SIZE)*(i-1));
			//so we dont go out of bounds
			if(i != num_pages_needed -1){

				ptr ->next_page  = find_page(DRAM + OSLAND + sysconf(_SC_PAGE_SIZE)*(i+1));
			} 
			else{
				ptr ->next_page = NULL;
			}

			if(ptr != start){
				ptr->virtual_addr = ptr->memBlock;
				ptr->owner = scheduler ->current;
				ptr ->space_remaining = 0;
				ptr ->is_initialized = true;
				//if we are the last page
				if(ptr->next_page == NULL){
					ptr->space_remaining = sysconf(_SC_PAGE_SIZE)*num_pages_needed	- numRequested;

				}


			}


		}

			ptr = ptr->next_page;
	}
	return start;

}

/* 
 *@param numRequested: number of bytes of Requested USER requested 
 * *page_alloc takes in a pointer to a page and allocates the appropriate amount
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
	if(usable_space != NULL && !os_mode){

		curr_page ->space_remaining -= (numRequested + 4);
	}
	return usable_space;//mallocDetails(numRequested, thatSoMeta);	
}
/**
 * If a malloc is odd , give them an even amount
 * also makes sure it is within bounds of OSland or USR LAND
 *
 * */
size_t validateInput(page * curr_page, size_t numRequested,bool os_mode)
//PUT THIS THIS IN THE WRAPPER FUNCTION
{
	int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND-4;
	//must be within array bounds
	if (numRequested <= 0 ||(int) numRequested > maxSize)
		{
			printf("INVALID REQUEST, CANNOT ALLOC\n");
			return 0;
		}

	//allocation must be even
	numRequested += numRequested %2;
	//this is for the case that user requests more than a page(taking metadata into account)
	if (((int)numRequested+4) > sysconf(_SC_PAGE_SIZE))
	{
		printf("I HAVE BEEN CALLED!\n");
		//this is the number of bytes taking up but not necessarily filling the last page
		int overflow = (numRequested+4)%sysconf(_SC_PAGE_SIZE);
		//in case user uses a very small number of bytes on the last page,so that when it is freed there may either be not enough space for a metadata block, or just enough for a metadata block but no user data between it and the next block
		if (overflow >0 && overflow < 5)
		{
			numRequested += 4;
		}
	}

	return numRequested;
}


/**
	returns pointer to METADATA BLOCK of first incidence of sufficiently large block
 *@param curr_page a empty page or a page a thread owns with enough space
 @param numRequested number of blocks to be allocated
 *@param os_mode an OSMODE flag which tells page_alloc to allocate in osland,curr_page should be null and is ignored if this flag is set to true
*/

char* findSpace(page * curr_page, int numReq,bool os_mode)
{
	//tracks how far down the array has been traveled
	int consumed = 0;
	//keeps trace of value contained in current metadata block
	void * currMeta;
	if(os_mode){
		currMeta = DRAM;

		*(int *) currMeta = *(int *)DRAM;
	}	
	else{
		 currMeta = curr_page -> memBlock;
		*(int *)currMeta = *(int *)curr_page -> memBlock;
	}
		//ternary operator to determine max size if OSLAND then it gets osland size, else the blocks capcity	
		int maxSize = (os_mode == false) ? curr_page -> capacity : OSLAND;
		//finds the next metadata block
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
/**
 *This defrags within a page or group of contigous pages, basically systems defrag 
 *@param curr_page: the start of the contigous block you want to defrag 
 @param os_mode: if os mode is true we currently do nothing
 *
 * */
void defrag (page * curr_page,bool os_mode)
{
	//currently we dont defrag in os_mode
	if(os_mode)
	{
		return;
	}
	int consumed = 0;
	char * home = (char *) (curr_page -> memBlock);
	char * probe = (char *) (curr_page -> memBlock);
	//proceed until the end of the memBlock is reached
	while(consumed < curr_page -> capacity)
	{
		//proceed until landing upon first empty block
		while (consumed < curr_page -> capacity  && (*(int *)home)%2 == 1)
		{
			//update consumed by amount of space after metadata block, plus size of metadata block itself
			consumed += 4+*(int *)home;

			home += (4-1)+*(int *)home;
			probe += (4-1)+*(int *)probe;
		}

		probe += (4-1)+*(int *)probe;

		//continue hopping along memBlock and consolidating free blocks until you hit a non-free block
		while(consumed<curr_page -> capacity && (*(int *)probe)%2==0)
		{
				*(int*)home += 4+*(int *)probe;
				consumed += 4+*(int *)probe;
				probe += 4+*(int *)probe;
		}
		if (*(int *)(probe) %2 == 1)
		{
			home = probe;
		}

	}
}
/**
 *Gives a new page to the current context
 *
 * */
page *giveNewPage()
{
	int numOfPages = (DRAM_SIZE-OSLAND)/(sysconf(_SC_PAGE_SIZE));
	int i;
	for(i=0;i<numOfPages;i++){

		if(PT->pages[i]->is_initialized == false){
				PT->pages[i]->is_initialized = true;
				page_init(PT->pages[i]);
				//TODO should pick a victim and swap in a new page from swap
				//set owner equal to current
				PT->pages[i]->owner  = scheduler -> current;
				//set virtualadess = memBlock, this is the VA is what user acess, so basically VA == Inital adress
				PT->pages[i]->virtual_addr = PT->pages[i]->memBlock;
				return PT->pages[i];

			}
	}
		//TODO grab from swap if there are no free pages 
		printf("page pageTable is full :<\n");
		PT->freePages --;
		return NULL;	
}


/**
 *@param numReq number of bytes passed requested
 @param the block of memory operating os (for OSmod its osland)
 *Helper function to update metadatas
	if the amount a user requests+its metadata leaves remaining bytes too small to be their own metadata, those extra bytes are consolidated into the amount the user is given
 *
 * */
void* mallocDetails(int numReq, char* memBlock)
{
	int total = *(int*)memBlock;	
	//numLeft is the amount of USABLE space remaining - that is, once you've taken into account the remainder's metadata block
	int num_left = total - (numReq+4);
	
	if (num_left >= 0)
	{
		char * leftover_metadata = memBlock+4+numReq;
		*(int *)leftover_metadata = num_left;
		*(int*)memBlock = numReq+1;
	}
	else
	{
		*(int *) memBlock +=1;
	}
	return (void*)(memBlock+4*sizeof(char));
}


/******************* FREE-Y THINGS *******************/

/**
 *free function that os/our library uses user do not touch this at all
 * */
bool os_free(void * target)
{
	return page_free(target, true);
}
/**
 *Free() function that the USER will call
 *@param target: the pointer you want to free (does not have to be the start of a page)
 *
 * */
bool my_free(void * target)
{
	__atomic_store_n(&(scheduler->SYS),true,__ATOMIC_SEQ_CST);
	bool freed = page_free(target, false);
	if (freed == true)
	{
		//finds the page the target lies in
		page * curr_page = find_page(target);
		int numFreed = *(int *)(target -4);
		if (numFreed > (sysconf(_SC_PAGE_SIZE)-4))
		{
			page * start_page = curr_page;
			int numPages = numFreed/((sysconf(_SC_PAGE_SIZE))-4);
			int i;
			for (i=0; i<numPages; i++)
			{
				curr_page -> space_remaining = sysconf(_SC_PAGE_SIZE);
				curr_page = curr_page -> next_page;
			}
            if(curr_page -> next_page == NULL)
            {
                int next = *(int*)(start_page->memBlock);
                int np = (*(int*)(start_page->memBlock+4+next));
				//last page has no data
                if(np % 2 == 0)
                {
                    curr_page-> space_remaining = sysconf(_SC_PAGE_SIZE);
					
					
                }
                else
                {
					curr_page ->prev_page = NULL;
					//got the next metadata and subtract the start oof the first page and it should give u the diff
                    int metaFront = (int)(start_page->memBlock+4+next - curr_page->memBlock);
					metaFront -= 4;
					*(int*)curr_page->memBlock = metaFront;
                		}
            		}
						page_clean(start_page);	
		    	}
		else
		{
			//add 4 because space +meta is now free 
			curr_page -> space_remaining += *(int *)(target-4)+4;
			page_clean(curr_page);
		}

		__atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
		return true;
	}

	__atomic_store_n(&(scheduler->SYS),false,__ATOMIC_SEQ_CST);
	return false;
}

//return boolean true for success and failure
/**
 *@param target  target pointer
 @param os_mode flag if we are in os_mode or not
 the main function our wrapper function will call to free a block,
 free() calls os_mode = false with a page and if within a page, we calcualte the apporpate page
 os_free is for the OS ONLY and it works like system free expect with a bigger chunk
 *
 **/
bool page_free(void * target, bool os_mode)
{
	//calc boundaries	
	void* targetMeta = target - 4;
	if (os_mode == true)
	{
		if (targetMeta >= (void *)DRAM && targetMeta < (void *)(DRAM+OSLAND-5))
		{
			if (segment_free(targetMeta) == true)
				return true;
		}
	}
	else
	{
		if (segment_free(targetMeta) == true)
		{
			return true;
		}
	}

	//check that the section you're trying to free is within os space/user page bounds
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
	return false;
}



/***************** MULTIPLE PAGEY THINGS *****************/

int swap(page * p1, page * p2)
{
    int pageSize = sysconf(_SC_PAGE_SIZE);
    char temp[pageSize];
	printf("Trying to acess data %d",*(int *)p1->memBlock);
    memcpy(temp,p1->memBlock,pageSize);
    memcpy(p1->memBlock,p2->memBlock,pageSize);
    memcpy(p2->memBlock,temp,pageSize);
    void * t = p1->memBlock;
    p1->memBlock = p2->memBlock;
    p2->memBlock = t;
    int p1Info = getKey(p1->memBlock);
    int p2Info = getKey(p2->memBlock);
    if(p1Info == -1 || p2Info == -1)
    {
        printf("Error in swap, virtual Address CANNOT be found!\n");
        return -1;
    }
    page * tempPtr = PT->pages[p1Info];
    PT->pages[p1Info] = PT->pages[p2Info];
    PT->pages[p2Info] = tempPtr;
	return 0;
    
}
/**
 *Frees up page structs for pages that have been contigously allocated so that they can be used by diff contexts
 *@param start: start of a contigous chunk, the next chunk is start->next_page
 * */
void page_clean(page *start)
{
	//first clear out the linked list 
		page * temp = NULL;
		page* old_start = start;
		int distanceToMeta = *(int *)old_start->memBlock;
		if(start->next_page == NULL && start->space_remaining ==  start->capacity){
				start->is_initialized = false;
				start->owner = NULL;
		}
		else{
		while(start  != NULL){
			if(old_start ->memBlock+4+distanceToMeta %2 != 0 && start ->next_page == NULL){

					return;
				}
			if(sysconf(_SC_PAGE_SIZE) == start -> space_remaining){
			
				temp = start->next_page;
				start -> is_initialized = false;
				start -> owner = NULL;
				start ->next_page = NULL;
				start -> prev_page = NULL;
				start = temp;
				PT->freePages +=1;
			
			}
			else{
					start = start->next_page;
			}
		}
	}
}

/**
 *String method to print out the Page Table and its contents
 *@param start and end: because there are a lot of pages, start and end represent what range of pages to look at in the page table. 
 */
void  page_table_string(int start, int end)
{
    if(start > end)
    {
        printf("Incorrect Parameters\n");
    }
    char * ret;
    int i;
    for(i = start; i <= end; i++)
    {
     page * curr_page = PT->pages[i];
     printf("__________________________________________________________________\n");
     printf("Page Number: %d\n",i);
     printf("Prev Page Address: %p\n",curr_page->prev_page);
     printf("Next Page Address: %p\n",curr_page->next_page);
     printf("Owner Thread: %p\n",curr_page->owner);
     printf("Space Remaining: %d\n",curr_page->space_remaining);
     printf("Capacity: %d\n",curr_page->capacity);
     printf("Initialized State: %d\n",curr_page->is_initialized);
     printf("Mem Block Address: %p\n",curr_page->memBlock);
     printf("Virtual Address: %p\n",curr_page->virtual_addr);
     printf("__________________________________________________________________\n");
    }
}





