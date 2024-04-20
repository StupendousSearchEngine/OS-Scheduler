#ifndef PROCESS_H
#define PROCESS_H
#include"headers.h"
struct Process{
int id,arrival_time,run_time,priority,remaining_time,finish_time,response_time;
//I'm gonna add something for RR sake but we may remmove it later
int last_run_time;
pid_t process_id;
// int mem_size ;


};
void copyProcess(struct Process *p1, struct Process p2)
{
    if(p1==NULL)
    {
        perror("error in copying");
        return ;
    }
    p1->arrival_time=p2.arrival_time;
    p1->finish_time=p2.finish_time;
    p1->id=p2.id;
    p1->last_run_time=p2.last_run_time;
    p1->priority=p2.priority;
    p1->process_id=p2.process_id;
    p1->remaining_time=p2.remaining_time;
    p1->response_time=p2.response_time;
    p1->run_time=p2.run_time;
}
#endif /* PROCESS_H */
