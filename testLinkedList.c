#include <stdio.h>
#include <stdlib.h>
#include "linkedList.h"


int main() {
    int c= 0;
    struct Process* p1 = (struct Process*)malloc(sizeof(struct Process));
    p1->runTime = 1;
    c++;

    struct Process* p2 = (struct Process*)malloc(sizeof(struct Process));
    p2->runTime = 2;

    c++;

    struct Process* p3 = (struct Process*)malloc(sizeof(struct Process));
    p3->runTime = 3;

    c++;

    struct LinkedList* list = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    list->head = NULL;

    c=5;

    insertAtEnd(list, p1);
    printf("%d",list->head->data->runTime);
    insertAtEnd(list, p2);
    insertAtEnd(list, p3);

    // Printing the run times of the processes
    printf("The first process has run time: %d\n", list->head->data->runTime);
    printf("The second process has run time: %d\n", list->head->next->data->runTime);
    printf("The third process has run time: %d\n", list->head->next->next->data->runTime);

    // Freeing allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(list);

    return 0;
}
