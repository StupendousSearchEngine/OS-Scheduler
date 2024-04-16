#include "headers.h"
#include "processStruct.h"
#include<string.h>
#include<stdlib.h>
void clearResources(int);
int msgq_id;
int scheduler_pid;

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
  

    key_t key_id;
    int send_val;
    FILE *key = fopen("keyfile", "r");
    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);


/*
TODO_1.1:
Reads the processes.txt file generated from test_generator.c
First it loops to count the number of processes in the file
then creates a dynamic array of size equal to the number of processes
then reads each value from the file and stores it in the process struct attributes
and adds the read process to array of processes
*/

//TODO_1.1
    FILE *fileptr;
    fileptr=fopen("processes.txt","r");
    if(fileptr==NULL)
    {
        printf("File does not exist!");
        return 1;
    }
    fscanf(fileptr,"%*[^\n]\n");
    int number_of_processes=0;
    while(!feof(fileptr))
    {
        int id,arrival,run,p;
        fscanf(fileptr,"%d\t%d\t%d\t%d",&id,&arrival,&run,&p);
        number_of_processes=id;
    }

    fclose(fileptr);
    fileptr=fopen("processes.txt","r");
    if(fileptr==NULL)
    {
        printf("File does not exist!");
        return 1;
    }
    struct Process *arr_of_processes=malloc(number_of_processes*sizeof(struct Process));
    fscanf(fileptr,"%*[^\n]\n");
    int i=0;
    while(!feof(fileptr))
    {
        int id,arrival,run,p;
        fscanf(fileptr,"%d\t%d\t%d\t%d",&id,&arrival,&run,&p);
        arr_of_processes[i].id=id;
        arr_of_processes[i].arrival_time=arrival;
        arr_of_processes[i].run_time=run;
        arr_of_processes[i].priority=p;
        arr_of_processes[i].remaining_time=run;
        i++;
    }
//End of TODO_1.1


/*
TODO_1.2:
Takes the scheduling algoritm name and attributes from the user
*/

//TODO_1.2
  char scheduling_algorithm_name[10];
    printf("Enter Scheduling algorithm name (HPF)/(SRTN)/(RR) : ");
    scanf("%s",scheduling_algorithm_name);
    int quantum;
    if(strcmp(scheduling_algorithm_name,"RR")==0)
    {
        printf("Enter quantum size: ");
        scanf("%d",&quantum);
    }
    printf("Enter ctx switch time: ");
    int ctx_switch_time;
    scanf("%d",&ctx_switch_time);
//End of TODO_1.2


/*
TODO_1.3:
Forks a process to execute the scheduler.c 
gives it the scheduling attributes and if it's RR it will be give 
the quantum size

Note:
sprintf --> converts int to string and stores it in a dummy char arry in this case
to send it as and argument to scheduler
*/

//TODO_1.3
    printf("Forking Scheduler..\n");
    
    int pid = fork();
    scheduler_pid=pid;
    if (pid == -1)                         //fork() returns -1 if forking failed
        perror("Error in forking scheduler process!!\n");
    else if (pid == 0) {
        printf("Scheduling..\n");
        system("gcc -o scheduler.out scheduler.c");
        scheduler_pid=getpid();
        printf("%d\n",scheduler_pid);
        char temp1[10],temp2[10],temp3[10];
        if (scheduling_algorithm_name == "RR")
        {    
            
            sprintf(temp1,"%d",number_of_processes);
            sprintf(temp2,"%d",quantum);
            sprintf(temp3,"%d",ctx_switch_time);
            char*args[]={scheduling_algorithm_name,temp1,temp3,temp2};        
            printf("%d",execv("./scheduler.out", args));
        }        
        else
        { 
            sprintf(temp1,"%d",number_of_processes);
            sprintf(temp3,"%d",ctx_switch_time);
            char*args[]={scheduling_algorithm_name,temp1,temp3};  
            execv("scheduler.out", args);
        }
    }



/*
Here the parent should be the one to fork a process to run clk.c
So the parent forks a process to run clk.c , then it initiates the clk

Don't get confused between the previous fork and the clock fork
The previous one is to execute scheduler.c then it is not going to
affect anything else since the condition says pid!=0 which means that only
the parent will execute the next lines and fork the clk

*/

    if(pid!=0)
    {
        int clkpid=fork();
        if(clkpid==-1)
        {
            perror("Error in forking clock process!!");
        }
        else if(clkpid==0)
        {
            char*args[]={"dummy"};      //dummy argument for execv as it has to take arguments
            system("gcc -o clk.out clk.c");
            execv("./clk.out",args);
        }
        else
        {
        //TODO_1.4
            initClk();
        //End of TODO_1.4
/*
Here we loop on the processes array and check the clk and if there is a process which arrived
at this point in time send it to scheduler if no process arrived at this time continue looping 
*/
    //TODO_1.6
        int process_arr_index=0;
        printf("%d\n",number_of_processes);
        while (process_arr_index < number_of_processes) {
            if (arr_of_processes[process_arr_index].arrival_time == getClk()) {
                    printf("YEAH!!%d\n",getClk());
                    int send_val = msgsnd(msgq_id, &arr_of_processes[process_arr_index], sizeof(struct Process), !IPC_NOWAIT);
                    kill(scheduler_pid,SIGUSR1);
                    process_arr_index++;
        }
    }
    //End of TODO_1.6
    //TODO_1.7
    int stat_loc;
    waitpid(pid,&stat_loc,0);
    //End of TODO_1.7
        }
    }
    
//End of TODO_1.3
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.  //DONE
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}


// /*
// Clearing messge queue
// */

void clearResources(int signum)
{
    printf("CLEARING RESOURCES..\n");
    msgctl(msgq_id,IPC_RMID,(struct msqid_ds*)0);
    raise(SIGKILL);
}
