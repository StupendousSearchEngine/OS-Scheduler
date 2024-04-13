#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "processStruct.h"


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

    struct Queue* q = initQueue(0);

    push(q,p1);
    push(q,p2);
    push(q,p3);

    printf("The Top: %d\n", front(q)->runTime);
    pop(q);
    printf("The Top: %d\n", front(q)->runTime);
    pop(q);
    printf("The Top: %d\n", front(q)->runTime);
    
    

    // Printing the run times of the processes
    // printf("The second process has run time: %d\n", list->head->next->data->runTime);
    // printf("The third process has run time: %d\n", list->head->next->next->data->runTime);

    // Freeing allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(q);

    return 0;
}
