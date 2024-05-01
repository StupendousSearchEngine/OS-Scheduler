#include "headers.h"
#include "process.h"
#include<string.h>
#include<stdlib.h>
#include <time.h>

void delay(int number_of_seconds)

{

// Converting time into milli_seconds

int milli_seconds = 1000 * number_of_seconds;

// Storing start time

__clock_t start_time = clock();

// looping till required time is not achieved

while (clock() < start_time + milli_seconds) ;

}

struct msg_buffer {
    long msg_type;
int id,arrival_time,run_time,priority,remaining_time,finish_time,response_time;
//I'm gonna add something for RR sake but we may remmove it later
int last_run_time;
pid_t process_id;
};

void clearResources(int);
int msgq_id;
int scheduler_pid ;

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
    // FILE *key = fopen("keyfile", "r");
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
        arr_of_processes[i].wait=0;
        arr_of_processes[i].turn_around_time=0;
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
    if (pid == -1) {
        perror("Error in forking scheduler process!!\n");
    } else if (pid == 0) {
        printf("Scheduling..\n");
        printf("Scheduler process not running\n");

        // Compile the scheduler.c to scheduler.o
        if (system("gcc scheduler.c -o scheduler.o -lm") != 0) {
            perror("Error compiling scheduler.c\n");
            exit(EXIT_FAILURE);
        }
        printf("Scheduler process  running\n");

        scheduler_pid = getpid();
        printf("SCHEDULERPID from the generator: %d\n", scheduler_pid);

        char temp1[10], temp2[10], temp3[10];
        sprintf(temp1, "%d", number_of_processes);
        sprintf(temp3, "%d", ctx_switch_time);

        if (strcmp(scheduling_algorithm_name, "RR") == 0) {
            sprintf(temp2, "%d", quantum);
            char *args[] = {"scheduler.o", scheduling_algorithm_name, temp1, temp3, temp2, NULL};
            execv("./scheduler.o", args);
            perror("execv failed");
        } else {
            char *args[] = {"scheduler.o", scheduling_algorithm_name, temp1, temp3, NULL};
            execv("./scheduler.o", args);
            perror("execv failed");
        }
        printf("SCHEDULER STARTEDDDDDDDDDDDDD\n");
    } 

/*
Here the parent should be the one to fork a process to run clk.c
So the parent forks a process to run clk.c , then it initiates the clk

Don't get confused between the previous fork and the clock fork
The previous one is to execute scheduler.c then it is not going to
affect anything else since the condition says pid!=0 which means that only
the parent will execute the next lines and fork the clk

*/

    if (pid != 0) {
        sleep(5);
        int clkpid = fork();
        if (clkpid == -1) {
            perror("Error in forking clock process!!");
        } else if (clkpid == 0) {
            char *args[] = {"dummy", NULL}; // Dummy argument for execv as it has to take arguments
            if (system("gcc -o clk.out clk.c") != 0) {
                perror("Error compiling clk.c\n");
                exit(EXIT_FAILURE);
            }
            execv("./clk.out", args);
            perror("execv failed");
        } else {
            // Initialize clock
            initClk();

            // Loop through the processes array and check the clock
            // If a process has arrived at this point in time, send it to the scheduler
            // If no process arrived at this time, continue looping
            int process_arr_index = 0;
            printf("%d\n", number_of_processes);
            while (process_arr_index < number_of_processes) {
                int send_signal = 0;

                while (process_arr_index<number_of_processes && arr_of_processes[process_arr_index].arrival_time <= getClk()) {
                    printf("process arrived at clk %d\n", getClk());
                    //printf("SCHEDULERPID: %d\n", scheduler_pid);
                    struct msg_buffer message;
                    message.arrival_time = arr_of_processes[process_arr_index].arrival_time;
                    message.finish_time = arr_of_processes[process_arr_index].finish_time;
                    message.id = arr_of_processes[process_arr_index].id;
                    message.last_run_time = arr_of_processes[process_arr_index].last_run_time;
                    message.priority = arr_of_processes[process_arr_index].priority;
                    message.process_id = -1;
                    message.remaining_time = arr_of_processes[process_arr_index].remaining_time;
                    message.response_time = arr_of_processes[process_arr_index].response_time;
                    message.run_time = arr_of_processes[process_arr_index].run_time;
                    message.msg_type = 1;
                    int send_val = msgsnd(msgq_id, (void *)&message, sizeof(struct msg_buffer) - sizeof(long), 0);
                    if (send_val == -1) {
                        // Failed to send message
                        perror("msgsnd failed");
                    } else {
                        // Message sent successfully
                        printf("Message sent successfully\n");
                    }
                    send_signal = 1;
                    // kill(scheduler_pid, SIGUSR1);
                    // delay(1);
                    process_arr_index++;
                }
                if(send_signal != 0){
                    struct msg_buffer message;
                    message.arrival_time = -1;
                    int send_val = msgsnd(msgq_id, (void *)&message, sizeof(struct msg_buffer) - sizeof(long), 0);
                    if (send_val == -1) {
                        // Failed to send message
                        perror("msgsnd failed");
                    } else {
                        // Message sent successfully
                        printf("end queue process send successfully\n");
                    }
                    kill(scheduler_pid,SIGUSR1);
                } 
            }

            // End of loop

            // End of clock process forking
        }
    }

    int returnStatus;
    printf("this is pid %d\n",scheduler_pid);
    waitpid(scheduler_pid, &returnStatus, 0);
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