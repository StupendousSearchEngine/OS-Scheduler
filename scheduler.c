#include "headers.h"
#include "processStruct.h"

int ctx_switch_time;
int quantum;
struct Process current_process;
struct Process receive_process()
{
    key_t key_id=ftok("keyfile.txt",65);
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    struct Process received_process;
    int recv=msgrcv(msgq_id,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(recv==-1)
    {
        perror("Error receiving process!\n");
    }
    return received_process;
}

void handler(int signum)
{
    printf("Receiving process at time =%d\n",getClk());
    signal(SIGUSR1,handler);
}


void HPF()
{

}

void SRTN()
{

}

void RR()
{

}

int main(int argc, char * argv[])
{
    //TODO implement the scheduler :)
    //upon termination release the clock resources.

    initClk();
    signal(SIGUSR1,handler);
    printf("Hell Yeah %s\n",argv[0]);
    while(1);

}