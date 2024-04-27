#include "headers.h"
#include "process.h"
#include <string.h>
#include"queue.h"
#define SHM_SIZE 4096


struct msg_buffer {
    long msg_type;
int id,arrival_time,run_time,priority,remaining_time,finish_time,response_time;
//I'm gonna add something for RR sake but we may remmove it later
int last_run_time;
pid_t process_id;
};

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
void pickSchedulingAlgo();
struct Process *current_process;

struct Queue *ready_queue;
struct Queue *termList;
int scheduling_algo; //0 for RR //1 for  SRTN //2 for HPF
int key_id;
int msgq_id ;
char *shmaddr_for_process;
int shmid_for_process;

void decode_with_hash(const char* str, int* num1, int* num2) {
    // Tokenize the input string based on the '#' separator
    char* token = strtok((char*)str, "#");

    // Convert the first token to an integer
    if (token != NULL) {
        printf("first token %s",token);
        *num1 = atoi(token);
        printf("num1 %d",*num1);
    } else {
        // If the token is NULL, something went wrong
        printf("Error: Could not extract first number\n");
        exit(1);
    }
    printf("decoderremtime %d",*num1);
    // Get the next token (the second number)
    token = strtok(NULL, "#");

    // Convert the second token to an integer
    if (token != NULL) {
        *num2 = atoi(token);
    } else {
        // If the token is NULL, something went wrong
        printf("Error: Could not extract second number\n");
        exit(1);
    }
}

void receive_process()
{
    
    struct msg_buffer message;
    message.arrival_time = 0;
    int recv=msgrcv(msgq_id,(void *)&message,sizeof(struct msg_buffer) - sizeof(long),1,0);
    while(message.arrival_time!=-1){
        struct Process *recieved_process = NULL;
        recieved_process= (struct Process*) malloc(sizeof(struct Process));
        if(recv==-1)
        {
            perror("Error receiving process!\n");
        }
        recieved_process->arrival_time =  message.arrival_time;
        recieved_process->remaining_time =  message.remaining_time;
        recieved_process->priority =  message.priority;
        recieved_process->id =  message.id;
        recieved_process->process_id =  message.process_id;
        push(ready_queue, recieved_process);
        printQueue(ready_queue);
        printf("process with id %d pushed",recieved_process->id);
        int recv=msgrcv(msgq_id,(void *)&message,sizeof(struct msg_buffer) - sizeof(long),1,0);
    }
    
}

void handler(int signum)
{
    receive_process();
    if (scheduling_algo!=0)
        pickSchedulingAlgo();
    printf("end handler\n");
    signal(SIGUSR1,handler);
}

void processHandler(int signum)
{

    printf("the process send the scheulder a sig or RR\n");
    int rem_time;
    int clk_in_process;
    if (current_process)
    {
        decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);

        if(rem_time == 0) {
            current_process->finish_time = clk_in_process;
            current_process->remaining_time = 0;
            push(termList,current_process);
            current_process=NULL;
            raise(SIGXCPU);
            printf("curent process made to be null\n");
        }
        
    }
    printf("IN Process Handler\n");
  
    pickSchedulingAlgo();
    printf("end handler\n");
    signal(SIGUSR1,handler);
}


void runProcess(struct Process *top)
{
    printf("RUNNIG PROCESS....\n");
    if(!top)
    {
        printf("top process is null\n");
        return ;
    }
    printf("running process with id:%d\n", top->id);
    printf("process.remainingTime%d",top->remaining_time);
    printf("process id %d\n",top->process_id);

    if(top->process_id == -1)
    { // i have never forked this process before
        int pid = fork();
        if(pid==0){
            if (system("gcc process.c -o process.o") != 0) {
                perror("Error compiling process.c\n");
                exit(EXIT_FAILURE);
            }
            else if (pid==-1)
            {
                perror("Error in forking new process\n");
                exit(-1);
            }
            char temp1[10];
            sprintf(temp1, "%d", top->remaining_time);
            char *args[] = {"process.o", temp1, NULL};
            execv("./process.o", args);
            perror("execv failed");
        }
        else{
            printf("popping id %d", ready_queue->front->data->id);
            top->process_id = pid;
            current_process = top;
           
            pop(ready_queue);
            printQueue(ready_queue);
        }
    }
    else
    { // i have forked this process before
        fflush(stdout);
        printf("I saw you before!!!!\n");
        current_process = top;
        if (kill(top->process_id, SIGCONT) == -1) {
            perror("Error sending SIGCONT signal");
            printf("errormsg");
            exit(-1);
            
        } 
        else 
        {
            printf("after");
            printf("popping id %d", ready_queue->front->data->id);
            pop(ready_queue);
            printQueue(ready_queue);
        }
        
    }


}
void contextSwitch(int signum)
{
    printf("cntc SWUUUU\n");
    int currclk=getClk();
    while(currclk+ctx_switch_time!=getClk());
    signal(SIGXCPU,contextSwitch);
   

}



void HPF()
{
   struct Process *top=NULL;
    if (ready_queue&& ready_queue->front)
        top=ready_queue->front->data;
    if(!current_process && top)
    {
    
        runProcess(top);

    }
}
void SRTN()
{
    struct Process *top=NULL;
    
    printf("calling SRTN\n");
    if (ready_queue&& ready_queue->front)
        top=ready_queue->front->data;
  
    if (top)
    {
        int rem_time;
        int clk_in_process;
        if (current_process && top->remaining_time<current_process->remaining_time)
        {
            decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);
            ////////////////////////////////////////////////////////////////////  
            current_process->remaining_time=rem_time;
            int sigterm_indcator = kill(current_process->process_id,SIGSTOP);
            if(sigterm_indcator==-1)
            {
                perror("error in stopping process!!!\n");
                exit(-1);

            }
            if (current_process->remaining_time!=0)
            {
                push(ready_queue, current_process);
                printQueue(ready_queue);
            }
            current_process=NULL;
            raise(SIGXCPU);
            runProcess(top);
        }
        else if (!current_process)
            runProcess(top);
    }
    

}

void RR()
{
    struct Process *top=NULL;
    
    printf("calling RR\n");
    if (ready_queue&& ready_queue->front)
        top=ready_queue->front->data;
    if (current_process)
    {
        int rem_time;
        int clk_in_process;
       
        decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);
        ////////////////////////////////////////////////////////////////////  
        current_process->remaining_time=rem_time;
        int sigterm_indcator = kill(current_process->process_id,SIGSTOP);
        if(sigterm_indcator==-1)
        {
            perror("error in stopping process!!!\n");
            exit(-1);

        }
        if (current_process->remaining_time!=0)
        {
            printf("pushing process with remaining time %d tnito q",current_process->remaining_time);
            push(ready_queue, current_process);
            printQueue(ready_queue);
        }
        current_process=NULL;
        raise(SIGXCPU);
        runProcess(top);
    }
    else 
    {
        printf("current process is null\n");
        runProcess(top);
    }
    

}
void pickSchedulingAlgo()
{
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

int main(int argc, char * argv[])
{
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    printf("Scheduler main\n");
    int key = ftok("shmkey",70);
    shmid_for_process = shmget(key,SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid_for_process == -1) {
        perror("shmget failure");
        exit(1);
    }

    shmaddr_for_process = (char *)shmat(shmid_for_process, (void *)0, 0);
    if (shmaddr_for_process == (char *)(-1)) {
        perror("shmat failure");
        exit(1);
    }

    printf("helllo from the scheduler\n");
    key_id=ftok("keyfile",65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    signal(SIGUSR1,handler);
    signal(SIGUSR2,processHandler);
    signal(SIGXCPU,contextSwitch);
    initClk();
    termList = initQueue(scheduling_algo);
    printf("verfy Pid %d\n",getpid());
    current_process=NULL;
 
    ctx_switch_time=atoi(argv[3]);
    num_processes_total = atoi(argv[2]);
    
    if (argc==5)
        quantum= atoi(argv[4]);
    //add a queue for scheduling

    if (strcmp(argv[1],"RR")==0)
    {
        scheduling_algo=0;
        ready_queue= initQueue(scheduling_algo);
   
    }
    else if (strcmp(argv[1],"SRTN")==0)
    {
        scheduling_algo=1;
        ready_queue= initQueue(scheduling_algo);
       
    }
    else if (strcmp(argv[1],"HPF")==0)
    {
        scheduling_algo=2;
        ready_queue= initQueue(scheduling_algo);
    }
    else{
        perror("Please Enter a Valid Algo Name\n");
        exit(-1);
    }
    int prevclk=getClk();
    printf("RR QUANTUM %d\n",quantum);
    while (scheduling_algo==0)
    {
        
        while(prevclk+quantum!=getClk());
        prevclk=getClk();
        printf("clockking\n");
        RR();
        
    }
    while(1);

}