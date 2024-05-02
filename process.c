#include "headers.h"
#define SHM_SIZE 4096
int prev_time_seen;
void resume(int signum){
    prev_time_seen=getClk();
    signal(SIGCONT,resume);
}

/* Modify this file as needed*/
int remainingtime;

char *shmaddr_for_process;
int shmid_for_process;


char* concatenate_with_hash(int num1, int num2) {
    // Calculate the total length of the resulting string
    int total_length = snprintf(NULL, 0, "%d#%d", num1, num2) + 1; // +1 for the null terminator

    // Allocate memory for the resulting string
    char* result = (char*)malloc(total_length * sizeof(char));

    // Check if memory allocation was successful
    if (result == NULL) {
        printf("Memory allocation failed\n");
        exit(1); // Terminate program
    }

    // Construct the resulting string
    snprintf(result, total_length, "%d#%d", num1, num2);

    return result;
}



int main(int agrc, char * argv[])
{
    signal(SIGCONT,resume);
    initClk();
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

    prev_time_seen = getClk();
 
    remainingtime = atoi(argv[1]);
    printf("the procces with rem time %d forked\n",remainingtime);
    while (remainingtime > 0)
    {
        
     
        if(prev_time_seen != getClk()) {
            printf("Prev time in process : %d, Current clk: %d",prev_time_seen,getClk());
            remainingtime--;
            
            char str[5];
            sprintf(str, "%d", remainingtime);
            char* message = concatenate_with_hash(remainingtime,getClk());
            strcpy((char *)shmaddr_for_process, message);
            prev_time_seen = getClk();
            printf("remaining time%d\n",remainingtime);
            
        }

    }
//////////////////////////////////////
    char str[5];
    sprintf(str, "%d", remainingtime);
    char* message = concatenate_with_hash(remainingtime,getClk());
    strcpy((char *)shmaddr_for_process, message);
    printf("process finshed \n");
    kill(getppid(),SIGUSR2);
    

   destroyClk(false);
   printf("Process commiting self exit game\n");
   raise(SIGKILL);

    return 0;
}