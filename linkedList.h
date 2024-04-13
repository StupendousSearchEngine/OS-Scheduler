#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "processStruct.h"


struct ListNode {
    struct Process* data;
    
    struct ListNode* next;
};


struct LinkedList{
    struct ListNode* head;
};


struct LinkedList* initLikedList(){
    struct LinkedList* list = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    list->head = NULL;
    return list;
}

struct ListNode* insertNext(struct ListNode* head, struct Process* data) {

    if (!data) {
        printf("Error in inserting the next (the process is NULL)");
        return NULL;
    }

    struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
    
    newNode->data = data;
    newNode->next = NULL;

    if (head == NULL) {
        
        head = newNode;
    } else {
        newNode->next = head->next;
        head->next = newNode;
    }
    
    return newNode;
}

struct ListNode* removeBegin(struct LinkedList* list) {
    if (!list || !list->head) {
        list->head = NULL;
        return NULL;
    } else {
        struct ListNode* temp = list->head;
        list->head = list->head->next;
        free(temp);
        return list->head;
    }
}

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

bool removeNext(struct ListNode* node){
    if(!node || !node->next) return 0;
    else {
        struct ListNode* temp = node->next;
        node->next = node->next->next;
        free(temp);
        return 1;
    }
    return 1;
}

bool removeEnd(struct LinkedList* list){
    struct ListNode* head = list->head;
    if(!head){
        return 0;
    }
    else{
        struct ListNode* cur = head;
        while(cur->next->next){
            cur = cur->next;
        }
        struct ListNode* newNode = cur->next;
        removeNext(cur);
        free(newNode);
    }
    return 1;
}

bool getEnd(struct LinkedList* list){
    struct ListNode* head = list->head;
    if(!head){
        return 0;
    }
    else{
        struct ListNode* cur = head;
        while(cur->next->next){
            cur = cur->next;
        }
        struct ListNode* newNode = cur->next;
        cur->next = cur->next->next;
        free(newNode);
    }
    return 1;
}

#endif /* LINKEDLIST_H */
