#include "headers.h"
#include "process.h"
#include <string.h>
#include"queue.h"
 /*
    1. Start a new process. (Fork it and give it its parameters.)
    2. Switch between two processes according to the scheduling algorithm. (Stop the
    old process and save its state and start/resume another one.)
    3. Keep a process control block (PCB) for each process in the system. A PCB
    should keep track of the state of a process; running/waiting, execution time,
    remaining time, waiting time, etc.
    4. Delete the data of a process when it gets notifies that it finished. When a
    process finishes it should notify the scheduler on termination, the scheduler
    does NOT terminate the process.
    5. Report the following information
    (a) CPU utilization.
    (b) Average weighted turnaround time.
    (c) Average waiting time.
    (d) Standard deviation for average weighted turnaround time.
    6. Generate two files: (check the input/output section below)
    (a) Scheduler.log
    (b) Scheduler.perf
    */

int ctx_switch_time, num_processes_total;
int quantum;
void HPF();//temp
struct Process *current_process;
struct Queue *ready_queue;
int scheduling_algo; //0 for RR //1 for STRN //2 for HPF
void receive_process(struct Process *recieved_process)
{
    key_t key_id=ftok("keyfile.txt",65);
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    printf("HEREE\n");
    int recv=msgrcv(msgq_id,recieved_process,sizeof(struct Process),0,!IPC_NOWAIT);
    if(recv==-1)
    {
        perror("Error receiving process!\n");
    }
    recieved_process->process_id=-1;
    recieved_process->last_run_time=-1;
    printf("end recv\n");
    
}

void handler(int signum)
{
    printf("Receiving process at time =%d\n",getClk());
    
    struct Process *recieved_process= (struct Process*) malloc(sizeof(struct Process));
    if (!recieved_process)
    {
        perror("error in allocation");
        exit(-1);
    }
    receive_process(recieved_process);
    

    
 
    printf("??????????????????????????\n");

    if (!ready_queue||!recieved_process)
        printf("uuu");

    push(ready_queue, recieved_process);
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("READY QUEUE Top: %d\n",ready_queue->front->data->arrival_time);
    printf("end handler\n");
    HPF();//temp
    signal(SIGUSR1,handler);

}
void runProcess(struct Process *top)
{
    printf("RUNNIG PROCESS....\n");
    if(!top)
        return ;

    if(top!=current_process)
    {
        printf("sleeping for %d\n",ctx_switch_time);
        sleep(ctx_switch_time);
    }
    top->process_id=fork();
    if (top->process_id==-1)
    {
        perror("Error in forking new process\n");
        exit(-1);
    }
    free(current_process);
    current_process= malloc(sizeof(struct Process));
    if (!current_process)
    {
        perror("error in allocation");
        exit(-1);
    }
    printf("memory is correctly allocated\n");
  
    current_process=top;
    printf("IOIIOOI\n");
    current_process->last_run_time=getClk();
    printf("current_process %d\n",current_process->process_id);
    pop(ready_queue);
    if (current_process->process_id==0)//child
    {
       printf("child\n");
        //while(1);//CPU bound
        printf("running current process with id : %d",current_process->process_id);

    }
    else
    {
        printf("running current process with id : %d",current_process->process_id);
    }
         
}
void contextSwitch(int signum)
{
    printf("cntc SWUUUU\n");
    signal(SIGXCPU,contextSwitch);
    exit(1);

}



void HPF()
{
    
   struct Process *top=NULL;


    if (ready_queue&& ready_queue->front)
        top=ready_queue->front->data;
    if(!current_process&&top)
        runProcess(top);
    else if (current_process && (top && current_process->priority> top->priority))
    {
        kill(current_process->process_id,SIGXCPU);
        current_process->remaining_time-=(getClk()-current_process->last_run_time);
         printf("terminating current process with id : %d",current_process->process_id);
        current_process->process_id=-1;
        push(ready_queue,current_process);
        runProcess(top);

    }
}

void SRTN()
{
    struct Process *top;
    
    while(1)
    {
        top=ready_queue->front->data;
        if(!current_process||(top && top->remaining_time<current_process->priority))
        {
            /*cntx switch*/
            //pausing current process
            int sigterm_indcator = kill(current_process->process_id,SIGTERM);
            current_process->remaining_time-=(getClk()-current_process->last_run_time);
            current_process->process_id=-1;
            push(ready_queue,current_process);
            runProcess(top);
        }
    }
    

}

void RR()
{

}


int main(int argc, char * argv[])
{
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    signal(SIGUSR1,handler);
    signal(SIGXCPU,contextSwitch);
    initClk();
    printf("verfy Pid %d\n",getpid());
    ready_queue= initQueue(scheduling_algo);
    current_process=NULL;
    if(argc<5)
    

    ctx_switch_time=atoi(argv[3]);
    num_processes_total = atoi(argv[2]);
    
    //add a queue for scheduling
   
    if (strcmp(argv[1],"RR")==0)
    {
        scheduling_algo=0;
        RR();
    }
    else if (strcmp(argv[1],"STRN")==0)
    {
        scheduling_algo=1;
        SRTN();
    }
    else if (strcmp(argv[1],"HPF\0")==0)
    {
        scheduling_algo=2;
        printf("calling HPF\n");
        HPF();
    }
    else{
        perror("Please Enter a Valid Algo Name\n");
        exit(-1);
    }
    while(1)
    {
        int prevclk=getClk();
        while(getClk()==prevclk);
    

        switch (scheduling_algo)
        {
            case 0:
                RR();
                break;
            case 1:
                SRTN();
                break;
            case 2:
                HPF();
                break;
     
        }
        
    }

}