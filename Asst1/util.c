/**
 *Assuming we update the level beforehand
 * Enqueues onto a level
 *
 **/
#include "util.h"
/**
 *Enqueues a Node into the multi level queue. We assume the node has already been malloced the apprpirate level is already set
 *@param threadNode * Node  : A threadNode that has been malloced
 *
 **/

void enqueue(threadNode *Node){
    //check if the head if the queue is null and initialize relvant values
    threadQ* threadq = scheduler->tq[Node->qlevel];
    if(threadq == NULL){
        threadq = malloc(sizeof(threadQ));
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
    //front is empty similar if the list has not been malloced yet
    else if (threadq -> front == NULL) {
        thread_q_init(Node,threadq);
        threadq ->size ++;
    } 
    // add to the end of the Linked list
    else {
        threadq ->rear->next =(threadNode *) malloc(sizeof(threadNode));
        threadq ->rear ->next = Node;
        threadq->rear = Node;
        threadq -> rear ->next = NULL;
        threadq ->size ++;
    }
}


threadNode* dequeue () 
{
    int *curr;
	*curr = 0;
    threadQ* threadq = NULL;
    threadq = get_next_executable(curr);
    //Find the first threadQ that is non_empty
    //If NULL is returned, this means we either have nothing to Dequeue or and error has happened
    //Error could be you dequeued before you enqueued (ya idoit)
    if (threadq == NULL)
    {
        return NULL;
    }
    
	threadq->threshold -= 1;
    threadNode * tNode = threadq -> front;
    threadq -> front = threadq ->front->next;
    return tNode;
}
/**
 * Scans for non empty bucket in the multi level prioirty queue
 * @param int * curr takes a pointer to an iterator which is the current index in our Array of queues
 * 
 */


threadQ * get_next_executable(int * curr)
{
	threadQ non_empty = _scan_non_empty(curr);
	if (non_empty -> threshold == 0)
	{
		non_empty -> threshold = MAXTHD - *curr;
	}
	*curr += 1;
	if (*curr == LEVELS - 1)
	{
		return NULL;
	}
	non_empty = _scan_non_empty(curr);
}


threadQ * _scan_non_empty(int * curr){
    threadQ* threadq = NULL;
    do
    {
        threadq = scheduler->tq[*curr];
        *curr = *curr +1;
        if(*curr >= LEVELS){
            return NULL;
        }
    }
    while(threadq == NULL || (threadq!=NULL && threadq->front == NULL));
    return threadq;
}




void mutex_enqueue(threadNode * tNode, my_pthread_mutex_t * mutex)
{
	if (mutex->waitQ == NULL)
	{
		mutex->waitQ = (threadQ *)malloc(sizeof(threadQ));
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
		rear->next = (threadNode *)malloc(sizeof(threadNode));
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



void  thread_q_init(threadNode * tNode,threadQ* threadq){
        threadq -> front = (threadNode *)malloc(sizeof(threadNode));
        threadq ->rear = (threadNode *)malloc(sizeof(threadNode));
        threadq ->front = tNode;
        threadq ->rear = tNode;
        threadq ->front ->next = NULL;
        threadq -> rear ->next = NULL;
}
