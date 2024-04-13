#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include "processStruct.h"


struct ListNode {
    struct Process* data;
    
    struct ListNode* next;
};


struct LinkedList{
    struct ListNode* head;
};


void insertAtEnd(struct LinkedList* list, struct Process* data){
    struct ListNode* head = list->head;
    if(!head){
        head = (struct ListNode*)malloc(sizeof(struct ListNode));
        head->data = data;
        head->next = NULL;
        list->head = head;
    }
    else{
        struct ListNode* cur = head;
        while(cur->next){
            cur = cur->next;
        }
        struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
        cur->next = newNode;
        newNode->data = data;
        newNode->next = NULL;
    }
    return;
}

#endif /* LINKEDLIST_H */
