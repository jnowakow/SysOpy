#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

int catcherPid = -1;
int returnedCount = 0;
int toSendCount = 0;
//******************************
//Reception of returning signals
//******************************

void counter(int sig, siginfo_t *info, void *context){
    if(catcherPid != info->si_pid){
        printf("Ignoring signal not from catcher\n");
    }
    else
    {
        returnedCount++;
    }
    
}

void terminationHandler(int sig, siginfo_t *info, void *context){
    if(catcherPid != info->si_pid){
        printf("Ignoring end signal not from catcher\n");
    }
    else
    {
        printf("Number of returned signals: %d\nShould be: %d\n", returnedCount, toSendCount);
        exit(0);
    }
    

}

void setUpReception(int mode){
    sigset_t toBlock;
    sigfillset(&toBlock);
    sigdelset(&toBlock, SIGUSR1);
    sigdelset(&toBlock, SIGUSR2);
    sigdelset(&toBlock, SIGINT);
    sigdelset(&toBlock, SIGRTMIN);
    sigdelset(&toBlock, SIGRTMIN + 1);
    
    if(sigprocmask(SIG_BLOCK, &toBlock, NULL) != 0){
        perror("Error during setting blocked signals");
        exit(1);
    }
    
    struct sigaction actUSR1;
    actUSR1.sa_flags = SA_SIGINFO;
    sigfillset(&actUSR1.sa_mask);//block all other signals

    actUSR1.sa_sigaction = counter;

    struct sigaction actUSR2;
    actUSR2.sa_flags = SA_SIGINFO;
    sigfillset(&actUSR2.sa_mask);

    actUSR2.sa_sigaction = terminationHandler;

    switch(mode){
        case 0:
            if(sigaction(SIGUSR1, &actUSR1, NULL) != 0){
                perror("Error during establishing SIGUSR1 handler");
                exit(1);
            }
            if(sigaction(SIGUSR2, &actUSR2, NULL) != 0){
                perror("Error during establishing SIGUSR2 handler");
                exit(1);
            }
            break;
        case 1:
            if(sigaction(SIGUSR1, &actUSR1, NULL) != 0){
                perror("Error during establishing SIGUSR1 handler");
                exit(1);
            }
            if(sigaction(SIGUSR2, &actUSR2, NULL) != 0){
                perror("Error during establishing SIGUSR2 handler");
                exit(1);
            }
            break;
        case 2:
            if(sigaction(SIGRTMIN, &actUSR1, NULL) != 0){
                perror("Error during establishing SIGUSR1 handler");
                exit(1);
            }
            if(sigaction(SIGRTMIN + 1, &actUSR2, NULL) != 0){
                perror("Error during establishing SIGUSR2 handler");
                exit(1);
            }
            break;
        default:
            break;
    }

    
}

/*
******************
   Sending signals
******************
*/

void sendKill(){
    for(int i = 0; i < toSendCount; i++){
        if(kill(catcherPid, SIGUSR1) != 0){
            perror("Error durnig sending SIGUSR1");
            exit(1);
        }
    }
    
    if(kill(catcherPid, SIGUSR2) != 0){
            perror("Error durnig sending SIGUSR2");
            exit(1);
    }
}

void sendQueue(){
    union sigval val;

    for(int i = 0; i < toSendCount; i++){
        val.sival_int = i;
        if(sigqueue(catcherPid, SIGUSR1, val) != 0){
            perror("Error durnig sending SIGUSR1");
            exit(1);
        }
    }
    
    val.sival_int = toSendCount;

    if(sigqueue(catcherPid, SIGUSR2, val) != 0){
            perror("Error durnig sending SIGUSR2");
            exit(1);
    }

}

void sendRT(){
    for(int i = 0; i < toSendCount; i++){
        if(kill(catcherPid, SIGRTMIN) != 0){
            perror("Error durnig sending SIGUSR1");
            exit(1);
        }
    }
    
    if(kill(catcherPid, SIGRTMIN + 1) != 0){
            perror("Error durnig sending SIGUSR2");
            exit(1);
    }
}

int main(int argc, char **argv){
    if (argc != 4){
        printf("Wrong number of arguments\n");
        return 1;
    }

    //get catcherPid
    catcherPid = atoi(argv[1]);
    if(catcherPid <= 1){
        printf("Wrong catcher PID\n");
        return 1;
    }

    //get number of signals to send
    toSendCount = atoi(argv[2]);
    if(toSendCount <= 0){
        printf("Number of signals to send must be positve\n");
        return 1;
    }

    //set up reception of returnig signals
    
    if(strcmp(argv[3], "kill") == 0){
        setUpReception(0);
        sendKill();
    }
    else if(strcmp(argv[3], "sigqueue") == 0){
        setUpReception(1);
        sendQueue();
    }
    else if(strcmp(argv[3], "RT") == 0){
        setUpReception(2);
        sendRT();
    }
    else
    {
        printf("No such mode\n");
        return 1;
    }

    //wait for returnig signals;
    while (1)
    {
        pause();
    }
    
    




    return 0;
}