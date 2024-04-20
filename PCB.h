#ifndef PCB_H
#define PCB_H
#include "headers.h"
/*msh 3rfa hnst5dmha wla eh lsa*/
struct PCB{
    PCB *prev;
    PCB *next;
    int process_state; 
    pid_t process_id;
    struct Process *process; 

};
#endif /* PCB_H */
