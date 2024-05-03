#include "headers.h"
#include "process.h"
#include <string.h>
#include<math.h>
#include"queue.h"
#define SHM_SIZE 4096
void contextSwitch();
struct msg_buffer {
    long msg_type;
int id,arrival_time,run_time,priority,remaining_time,finish_time,response_time;
//I'm gonna add something for RR sake but we may remmove it later
int last_run_time;

pid_t process_id;
};
void RR();
int prevclk =-1;
int prev_remaining=-1;
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
int quantum=-1;
void pickSchedulingAlgo();
struct Process *current_process;
int terminated_process_number=0;
struct Queue *ready_queue;
struct Queue *termList;
int scheduling_algo; //0 for RR //1 for  SRTN //2 for HPF
int key_id;
int msgq_id ;
char *shmaddr_for_process;
int shmid_for_process;
FILE*scheduler_log;
FILE*scheduler_perf;
int total_time=0;
void decode_with_hash(const char* str, int* num1, int* num2) {
    // Tokenize the input string based on the '#' separator
    printf("IN DECODE WITH HASHHHH %s\n",str);
    fflush(stdout);
    char *temp = NULL;
    temp = (char *)malloc(strlen(str) + 1);
    if (temp == NULL) {
        perror("Memory allocation failed\n");
        exit(-1);
    }

    // Copy the string
    strcpy(temp, str);
    while(strlen(str)<2)
    {
        printf("WAITINGGGGGGGGGGGGGGGGGGG %s\n",str);
        fflush(stdout);
        sleep(1);
    }
  
    char* token = strtok((char*)temp, "#");
   
    if (token != NULL) {
     
        *num1 = atoi(token);
      
    } else {
        // If the token is NULL, something went wrong
        perror("Error: Could not extract first number\n");
        exit(-1);
    }
   
    // Get the next token (the second number)
    token = strtok(NULL, "#");

    // Convert the second token to an integer
    if (token != NULL) {
        *num2 = atoi(token);
    } else {
        // If the token is NULL, something went wrong
        perror("Error: Could not extract second number\n");
        exit(-1);
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
        recieved_process->run_time =  message.run_time;
        recieved_process->id =  message.id;
        recieved_process->process_id =  message.process_id;
        push(ready_queue, recieved_process);
        
        printf("process with id %d pushed\n",recieved_process->id);
        printQueue(ready_queue);
        int recv=msgrcv(msgq_id,(void *)&message,sizeof(struct msg_buffer) - sizeof(long),1,0);
    }
    
}

void handler(int signum)
{
    receive_process();
    if (scheduling_algo!=0 || scheduling_algo==0 && !current_process)
        pickSchedulingAlgo();
    
    signal(SIGUSR1,handler);
}
void rrPause(int signum)
{
    RR();
    signal(SIGXCPU,rrPause);
}
void processDone(int signum)
{
    printf("CURRENT PROCESS TERMINATING ID %d\n",current_process->id);
    fflush(stdout);
   
    int rem_time;
    int clk_in_process;
   
    if (current_process)
    {
        
        decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);
        
        if(rem_time == 0) {
            current_process->finish_time = clk_in_process;
            current_process->remaining_time = 0;
            current_process->turn_around_time=getClk()-current_process->arrival_time;
            fflush(scheduler_log);
            fprintf(scheduler_log,"At time %d process %d stopped arr %d total %d remain %d wait %d\n",current_process->finish_time,current_process->id,
            current_process->arrival_time,current_process->run_time,current_process->remaining_time,current_process->wait);
            fflush(scheduler_log);
            push(termList,current_process);
            printf("process with id %d pushed into termList\n",current_process->id);
            current_process=NULL;
            printQueue(termList);
            printf("QUEUE?\n");
            printQueue(ready_queue);
            current_process=NULL;
            contextSwitch();
            pickSchedulingAlgo();
        }
        
    }
    else 
    {
        printf("WEIRD\n");
    }
    printf("IN Process Handler\n");
  
    
   
    signal(SIGUSR2,processDone);
}


void runProcess()
{
    struct Process *top=NULL;
    //printf("In Running process function");
    
    
    if (current_process&&current_process->remaining_time>0)
    {
        printf("process with id %d pushed\n",current_process->id);
        push(ready_queue,current_process);
    }
   
    if (ready_queue&&ready_queue->size !=0)
        top=ready_queue->front->data;
    if(top)
    {
        if(top->process_id == -1) //new process
        {
            int pid = fork();
            ////////////////////////////
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
                char temp2[10];
                sprintf(temp2, "%d", quantum);
                char *args[] = {"process.o", temp1, temp2, NULL};
                execv("./process.o", args);
                perror("execv failed");
            }
            else{
               
             
                current_process = top;
                prev_remaining=current_process->remaining_time;
                prevclk=getClk();
                current_process->last_run_time=getClk();
                current_process->wait+=getClk()-current_process->arrival_time;
                fflush(scheduler_log);
                fprintf(scheduler_log,"At time %d process %d started arr %d total %d remain %d wait %d \n",getClk(),current_process->id,
                current_process->arrival_time,current_process->run_time,current_process->remaining_time,current_process->wait);
                fflush(scheduler_log);
                top->process_id = pid;
                printf("popping process with id %d in runprocess 1st else\n",top->id);
                pop(ready_queue);
                printQueue(ready_queue);
                
            }
        }
        else
        {
            printf("I saw you before!!!!\n");   
            current_process = top;
            current_process->wait += getClk()-current_process->last_run_time;
            current_process->last_run_time=getClk();
            fflush(scheduler_log);
            fprintf(scheduler_log,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),current_process->id,
            current_process->arrival_time,current_process->run_time,current_process->remaining_time,current_process->wait);
            fflush(scheduler_log);
            if (kill(top->process_id, SIGCONT) == -1) {
                perror("Error sending SIGCONT signal");
                exit(-1);
                
            } 
            else 
            {
                prevclk=getClk();
                prev_remaining=current_process->remaining_time;
                
                printf("popping process with id %d in runprocess second else\n",top->id);
                pop(ready_queue);
                printQueue(ready_queue);
            }
        }
    }
    
    
   
}
void contextSwitch()
{
    printf("cntc SWUUUU %d\n ", getClk());
    int currclk=getClk();
    while(currclk+ctx_switch_time!=getClk());
    printf("cntc SWUUUU AFTERRR %d \n", getClk());
}


void HPF()
{
    if(!current_process)
    {
            
        printf("Enter HPF...\n");
        runProcess();
       
    }
}
void SRTN()
{
    
    printf("calling SRTN\n");
   
  

    int rem_time;
    int clk_in_process;
    if (current_process)
    {
        int prev=getClk();
        decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);
        ////////////////////////////////////////////////////////////////////  
        current_process->remaining_time=rem_time;
        if(current_process->remaining_time>0)
        {
            int sigterm_indcator = kill(current_process->process_id,SIGSTOP);
            if(sigterm_indcator==-1)
            {
                perror("error in stopping process!!!\n");
                exit(-1);

            }
            //current_process=NULL;
            contextSwitch();
            
        }
     
    }
    runProcess();
    
    

}

void RR()
{
    //printf("calling RR\n");
    int rem_time;
    int clk_in_process;
    if (current_process)
    {
      
        decode_with_hash(shmaddr_for_process,&rem_time,&clk_in_process);
        current_process->remaining_time=rem_time;
        if(current_process->remaining_time>0)
        {
            int sigterm_indcator = kill(current_process->process_id,SIGSTOP);
            if(sigterm_indcator==-1)
            {
                perror("error in stopping process!!!\n");
                exit(-1);

            }
            contextSwitch();
            runProcess();
        }
        else if (current_process->remaining_time==0)
        {
            printf("process id %d needs a lil push",current_process->id);
            int size = termList->size;
            kill(current_process->process_id,SIGCONT);
            while(termList->size == size);
        }
     
    }
    else
    {
        runProcess();
    }
    
}
int total_execution_time=0;
double total_wta=0;
double total_waiting_time=0;
int*wta_arr;
void calculate_statistics()
{
    int i=0;
    printf("Evil sarah\n");

while(termList->size>0 && termList->front->data)
{

    total_execution_time+=termList->front->data->run_time;


    total_waiting_time=termList->front->data->wait;

printf("%d\n",termList->front->data->run_time);
    total_wta=(termList->front->data->turn_around_time)/(termList->front->data->run_time);


    wta_arr[i]=termList->front->data->turn_around_time;
    
    printf("popping process with id %d from term list\n",termList->front->data->id);
    pop(termList);
    printQueue(termList);
  
    i++;

}
}

void write_perf()
{
    printf("p1");
     calculate_statistics();
     fflush(scheduler_perf);
         printf("p2");

     fprintf(scheduler_perf, "CPU utilization = %0.2f%c \n",((float)total_execution_time/getClk())*100,'%');
         printf("p3");

     fprintf(scheduler_perf,"Avg WTA = %0.2f \n",total_wta/num_processes_total);
         printf("p4");

     double avg=total_wta/num_processes_total;
         printf("p5");

     fprintf(scheduler_perf,"Avg Waiting = %0.2f \n",total_waiting_time/num_processes_total);
         printf("p6");

     double sd=0;
         printf("p7");

     for(int i=0;i<num_processes_total;i++)
     {
         sd+=pow((wta_arr[i]-avg),2);
     }
         printf("p8");

      fprintf(scheduler_perf,"Std WTA = %0.2f \n",sd/num_processes_total);
          printf("p9");

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
    scheduler_log=fopen("schedulder.log","w");
    scheduler_perf=fopen("scheduler.perf","w");
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
    signal(SIGUSR2,processDone);
    signal(SIGXCPU, rrPause);
    initClk();
    termList = initQueue(0);
    printf("verfy Pid %d\n",getpid());
    current_process=NULL;
 
    ctx_switch_time=atoi(argv[3]);
    num_processes_total = atoi(argv[2]);
    wta_arr=malloc(num_processes_total*sizeof(struct Process));
    if(!wta_arr)
    {
        perror("Could not allocate memory\n");
        exit(-1);
    }
    
    if (argc==5)
        quantum= atoi(argv[4]);
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


    while(termList->size!=num_processes_total);

    write_perf();
    fclose(scheduler_perf);
    fclose(scheduler_log);
}