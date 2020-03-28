#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

pid_t senderPid;
int recivedSignals = 0;

void setUpReception(){
    struct sigaction act;

    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGUSR1);
    sigdelset(&act.sa_mask, SIGUSR2);


}



int main(int argc, char **argv){
    if (argc != 2){
        printf("Wrong number of arguments\n");
        return 1;
    }

    printf("Catcher's pid: %d", getpid());

    if(strcmp(argv[1], "kill") == 0){
        //kill mode
        setUpReception();
        resendKill();
    }
    else if (strcmp(argv[1], "sigqueue") == 0)
    {
        //sigqueue mode
    }
    else if (strcmp(argv[1], "RT") == 0)
    {
        //real time mode
    }
    else
    {
        printf("Wrong argument!\n");
        return 1;
    }

    printf("Sender PID: %d\n Number of received signals: %d\n", senderPid, recivedSignals);

    return 0;
}