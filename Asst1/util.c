/**
 *Assuming we update the level beforehand
 * Enqueues onto a level
 *
 **/
#include "util.h"
void enqueue(threadNode *Node){
    //check if the head if the queue is null
    threadQ* threadq = scheduler->tq[Node->qlevel];
    if(threadq == NULL){
        threadq = malloc(sizeof(threadQ));
        _thread_q_init(Node,threadq);
        threadq ->size =1;
        threadq -> min = scheduler->tq[Node->qlevel-1]->max;
        threadq->max = threadq->min + ( MULTIPLIER * Node->qlevel * 25000 );
        
    } 
    //front is empty similar if the list has not been malloced yet
    else if (threadq -> front == NULL) {
        _thread_q_init(Node,threadq);
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
    int curr = 0;
    threadQ* threadq = NULL;
    do 
    {
        threadq = scheduler->tq[curr];
        curr++;
    }
    while(threadq == NULL || threadq ->front == NULL);
    if (threadq == NULL)
    {
        return NULL;
    }
    threadNode * tNode = threadq -> front;
    threadq -> front = threadq ->front->next;
    return tNode;
}


void _thread_q_init(threadNode * tNode,threadQ* threadq){
        threadq -> front = (threadNode *)malloc(sizeof(threadNode));
        threadq ->rear = (threadNode *)malloc(sizeof(threadNode));
        threadq ->front = tNode;
        threadq ->rear = tNode;
        threadq ->front ->next = threadq->rear;
        threadq -> rear ->next = NULL;
}



void mutex_enqueue(threadNode * tNode, my_pthread_mutex_t * mutex)
{
	if (mutex->waitQ == NULL)
	{
		mutex->waitQ = (threadQ *)malloc(sizeof(threadQ));
		_thread_q_init(tNode, mutex->waitQ);
		mutex ->waitQ -> size = 1;
	}
	
	else if (mutex->waitQ->front == NULL)
	{
		_thread_q_init(tNode, mutex->waitQ);
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
threadNode * mutex_dequeue(my_pthread_mutex_t *mutex){
	if(mutex->waitQ ->size == 0){

		return NULL;
	} else{
			threadNode * ptr = mutex->waitQ->front;
			mutex->waitQ->front = mutex->waitQ->front->next;
			mutex->waitQ->size --;
			return ptr;
	}

}



