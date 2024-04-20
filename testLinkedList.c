#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "process.h"


int main() {
    int c= 0;
    struct Process* p1 = (struct Process*)malloc(sizeof(struct Process));
    p1->remaining_time = 3;
    c++;

    struct Process* p2 = (struct Process*)malloc(sizeof(struct Process));
    p2->remaining_time = 1;

    c++;

    struct Process* p3 = (struct Process*)malloc(sizeof(struct Process));
    p3->remaining_time = 2;

    c++;

    struct Queue* q = initQueue(1);

    push(q,p1);
    push(q,p2);
    push(q,p3);

    printf("The Top: %d\n", front(q)->remaining_time);
    pop(q);
    printf("The Top: %d\n", front(q)->remaining_time);
    pop(q);
    printf("The Top: %d\n", front(q)->remaining_time);
    pop(q);
    pop(q);
    
    free(p1);
    free(p2);
    free(p3);
    free(q);

    return 0;
}
