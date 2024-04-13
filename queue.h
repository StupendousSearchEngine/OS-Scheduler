#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include "linkedList.h"
#include "processStruct.h"


struct Queue{
    struct ListNode* front;
    struct ListNode* back;
    struct LinkedList* list;
    int size;
    int algo;
};

struct Queue* initQueue(int algo){
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));

    queue->list = initLikedList();
    queue->back = NULL;
    queue->front = NULL;
    queue->size = 0;
    queue->algo = algo;

    return queue;
}

bool push(struct Queue* queue, struct Process* data){
    if(!queue) return 0;
    else queue->back = insertNext(queue->back,data),queue->size++;
    if(queue->size == 1) queue->front = queue->back , queue->list->head=queue->front;
    return 1;
}

bool pop(struct Queue* queue){
    if(!queue || !queue->size) return 0;
    else {
        queue->front = removeBegin(queue->list);
        queue->size--;
        if(queue->size == 1) queue->back = queue->front;
    }
    return 1;
}

struct Process* front(struct Queue* queue){
    if(!queue || !queue->size) return NULL;
    else return queue->front->data;
}


#endif /* QUEUE_H */
