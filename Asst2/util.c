/**
 *Assuming we update the level beforehand
 * Enqueues onto a level
 *
 **/
#include "page_util.h"
int tCount=0;
int initScheduler(){
        scheduler = (Scheduler *)osmalloc(sizeof(Scheduler));
		scheduler->tq = (threadQ **)osmalloc(sizeof(threadQ *) * LEVELS);
		int i;
		for(i=0;i<LEVELS;i++){

			scheduler->tq[i] = NULL;
		}
		scheduler -> current = NULL;
		scheduler -> no_threads = 1;
		memset(&(scheduler->sa),0,sizeof(scheduler->sa));
		//memset fails
		scheduler ->sa.sa_handler = &normal_sig_handler;
		sigaction(SIGVTALRM,&scheduler->sa,NULL);
		scheduler->timer.it_value.tv_sec = 0;
		scheduler->timer.it_value.tv_usec = 25000;
		scheduler->current = NULL;
		scheduler->current = createNewNode(scheduler->current,0,1,0,NULL,NULL,NULL);
	    scheduler->SYS = false;
        init = 1;
        setitimer(ITIMER_VIRTUAL,&scheduler->timer, NULL);
        return 0;
}
threadNode * createNewNode(threadNode *node,int level,int numSlices,double spawnTime,my_pthread_t *thread,void*(*function)(void*),void * arg)
{
	ucontext_t newthread;
	//thread = osmalloc(sizeof(my_pthread_t));
	node = (threadNode *)osmalloc(sizeof(threadNode));
	node -> tid = tCount++;
	node -> next = NULL;
	node -> spawnTime = spawnTime; 
	node -> numSlices = numSlices;
	node -> qlevel = level;
	node -> waitingNodes = NULL;
	node -> term =0;
    node -> return_value = NULL;
	node->did_join = false;
	node->is_waiting = false;
	/**
	 * IF the argument is NULL that means we already have a context with a function, we dont have to call makecontext(), we use when we want to create a Node for the main thread
	 * **/

	getcontext(&newthread);
	//only create a new context if we pass in a new parameter
	if(function != NULL){
		
		*thread = node;
        
	    //we dont want anything to happen after thread is done
		newthread . uc_link = 0;
		newthread. uc_stack.ss_sp=osmalloc(MEM);
		newthread. uc_stack.ss_size=MEM;
 		newthread. uc_stack.ss_flags=0;
		makecontext(&newthread,(void*)wrapper_function, 2,function,arg);
		
	}
	
	//node ->thread = osmalloc(sizeof(ucontext_t*));
	
    node->thread = newthread;
	return node;
	
}


/**
 *Enqueues a Node into the multi level queue. We assume the node has already been osmalloced the apprpirate level is already set
 *@param threadNode * Node  : A threadNode that has been osmalloced
 *
 **/

void enqueue(threadNode *Node)
{
    //check if the head if the queue is null and initialize relvant values
    threadQ* threadq = scheduler->tq[Node->qlevel];
    if(threadq == NULL)
	{
		threadq = osmalloc(sizeof(threadQ));
		scheduler->tq[Node->qlevel] = threadq;	
		//shceduler->tq[0]->fronto
        thread_q_init(Node,threadq);
        threadq ->size =1;
        if(Node->qlevel == 0){
            threadq->min = 0;
        }
        else{
            threadq -> min = scheduler->tq[Node->qlevel-1]->max;
        }
        threadq->max = threadq->min + ( MULTIPLIER * Node->qlevel * 25000 );
        threadq->threshold = MAXTHD - Node->qlevel;
        
    } 
    //front is empty similar if the list has not been osmalloced yet
    else if (threadq -> front == NULL) 
	{
        thread_q_init(Node,threadq);
        threadq ->size ++;
    } 
    // add to the end of the Linked list
    else {
        threadq ->rear ->next = Node;
        Node->next = NULL; //ADDED: id=1
        threadq->rear = Node;
        //threadq -> rear ->next = NULL; REMOVED: id=1
        threadq ->size ++;
    	if (threadq -> threshold == 0)
		{
			//was something supposed to go here??
		}
	}
}
//i don't know if you even have to do anything special if you hit threshold and run out of nodes at the same time - 
//when you enqueue, the threshold is still zero. next time you come around you skip it, as you should, and reset the threshold to max

//NOTE: DEQUEUE ONLY AFTER YOU DEMOTE THE CURRENT NODE
//ALSO, CURR = QLEVEL OF CURRENT NODE
threadNode* dequeue (int  curr) 
{
    threadQ* threadq = get_next_executable(&curr);
    //If NULL is returned, this means we either have nothing to Dequeue or and error has happened
    //Error could be you dequeued before you enqueued (ya idoit)
    if (threadq == NULL)
    {
        return NULL;
    } 
    threadNode * tNode = threadq -> front;
    threadq->size--;
    threadq -> front = threadq ->front->next;
    tNode->next = NULL;
    return tNode;
}
/**
 * Scans for non empty bucket in the multi level prioirty queue
 * @param int * curr takes a pointer to an iterator which is the current index in our Array of queues
 * 
 */

threadQ * get_next_executable(int * curr)
{
	int  last;
	//find first non empty thread/Q
	threadQ * non_empty = _scan_non_empty(curr);
    if(non_empty == NULL){
        return NULL;
    }
    //if the size one this means that we will dq the same thread we just enqed so start from 0 again if this is the case
	//save the location of the first non empty queue in case of edge cases
	last = *curr;
	//if it's already maxed out its threshold, set threshold back to max and find next non empty
	if (non_empty -> threshold == 0)
	{
		non_empty -> threshold = MAXTHD - *curr;
		*curr += 1;
		if(*curr >= LEVELS){
				*curr = 0;
			}
	}
	non_empty = _scan_non_empty(curr);
	//THIS IS FOR THE EDGE CASE THAT THERE WAS ONLY ONE NONEMPTY QUEUE, AND IT HAPPENED TO HAVE BEEN MAXED OUT
	//if there are no other nonempty threadQs after the one we just had
	if (non_empty == NULL)
	{
		//look for another nonempty starting from the beginning
		non_empty = _scan_non_empty(&last);
		//set the threshold back to max, dequeue normally
		if (non_empty -> threshold == 0)
		{
			non_empty -> threshold = MAXTHD - *curr;
		}
	}
	if (non_empty == NULL)
		{
			return NULL;
		}
	non_empty->threshold -= 1;
	return non_empty;
}

threadQ * _scan_non_empty(int * curr)
{
    bool wrap = false;
    threadQ* threadq = scheduler->tq[*curr];
	while(threadq == NULL || (threadq ->front == NULL)){	
        *curr = *curr +1;     
        if(*curr >= LEVELS)
		{
            if(wrap){
                return NULL;
            }
            *curr = 0;
            wrap = true;
        }
        threadq = scheduler->tq[*curr];

	}
    return threadq;
}

void mutex_enqueue(threadNode * tNode, my_pthread_mutex_t * mutex)
{
	if (mutex->waitQ == NULL)
	{
		mutex->waitQ = (threadQ *)osmalloc(sizeof(threadQ));
		thread_q_init(tNode, mutex->waitQ);
		mutex ->waitQ -> size = 1;
	}
	
	else if (mutex->waitQ->front == NULL)
	{
		thread_q_init(tNode, mutex->waitQ);
		mutex ->waitQ ->size =1;
	}
	else
	{
		threadNode * rear = mutex->waitQ->rear;
		//rear->next = (threadNode *)osmalloc(sizeof(threadNode));
		rear->next = tNode;
		mutex->waitQ->rear = rear->next;
		mutex->waitQ->rear->next = NULL;
		mutex ->waitQ -> size ++;
	}
}

threadNode * mutex_dequeue(my_pthread_mutex_t *mutex)
{
	if(mutex->waitQ ->size == 0)
	{
		return NULL;
	} 
	else
	{
			threadNode * ptr = mutex->waitQ->front;
			mutex->waitQ->front = mutex->waitQ->front->next;
			mutex->waitQ->size --;
			return ptr;
	}

}

void  thread_q_init(threadNode * tNode,threadQ* threadq)
{
        threadq ->front = tNode;
        threadq ->rear = tNode;
        threadq ->front ->next = NULL;
        threadq -> rear ->next = NULL;
}
