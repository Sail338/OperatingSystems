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
        _enqueueHelper(Node,threadq);
        threadq ->size =1;
        threadq -> min = scheduler->tq[Node->qlevel-1]->max;
        threadq->max = threadq->min + (MULTIPLIER * Node->qlevel * 25000);
        
    } 
    //front is empty similar if the list has not been malloced yet
    else if (threadq -> front == NULL) {
        _enqueueHelper(Node,threadq);
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


void _enqueueHelper(threadNode *Node,threadQ* threadq){
        threadq -> front = (threadNode *)malloc(sizeof(threadNode));
        threadq ->rear = (threadNode *)malloc(sizeof(threadNode));
        threadq ->front = Node;
        threadq ->rear = Node;
        threadq ->front ->next = threadq->rear;
        threadq -> rear ->next = NULL;


}