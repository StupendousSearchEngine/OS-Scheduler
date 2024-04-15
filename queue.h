#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h> 
#include "processStruct.h"

struct ListNode {
    struct Process* data;
    
    struct ListNode* next;
};

struct Queue{
    struct ListNode* front;
    struct ListNode* back;
    int size;
    int algo; // 0:normal,  1:lowest remaining time on top,  2:lowest priority value on top
};

struct Queue* initQueue(int algo){
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->back = NULL;
    queue->front = NULL;
    queue->size = 0;
    queue->algo = algo;

    return queue;
}

bool push(struct Queue* queue, struct Process* data){
    if(!queue) return 0;
    struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
    newNode->data = data;
    newNode->next=NULL;
    if(queue->size == 0){
        queue->back = newNode;
        queue->front = newNode;
        queue->size = 1;
        return 1;
    }
    // Normal Queue
    if(queue->algo == 0){
        queue->back->next = newNode;
        queue->back = newNode;
        return 1;
    }

    else if(queue->algo == 1){
        struct ListNode* cur = queue->front;
        struct ListNode* prev = NULL;
        while(cur && cur->data->remaining_time < data->remaining_time){
            prev = cur;
            cur = cur->next;
        }
        if(!prev) {
            struct ListNode* temp = queue->front;
            queue->front = newNode;
            newNode->next = temp;
        }
        else{
            if(!prev->next) queue->back = newNode;
            struct ListNode* temp = prev->next;
            prev->next = newNode;       
            newNode->next=temp;
        }
    }

    else{
        struct ListNode* cur = queue->front;
        struct ListNode* prev = NULL;
        while(cur && cur->data->priority < data->priority){
            prev = cur;
            cur = cur->next;
        }
        if(!prev) {
            struct ListNode* temp = queue->front;
            queue->front = newNode;
            newNode->next = temp;
        }
        else{
            if(!prev->next) queue->back = newNode;
            prev->next = newNode;       
        }
    }
    queue->size++;
    return 1;
}

bool pop(struct Queue* queue){
    if(!queue || !queue->size) return 0;
    else {
        struct ListNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
        queue->size--;
        if(queue->size == 1) queue->back = queue->front;
        else if(queue->size == 0) queue->back = 0, queue->front=0;
    }
    return 1;
}

struct Process* front(struct Queue* queue){
    if(!queue || !queue->size) return NULL;
    else return queue->front->data;
}


#endif /* QUEUE_H */
