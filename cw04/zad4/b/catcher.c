#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

pid_t senderPid = -1;
int recivedSignals = 0;
int mode = -1;

//***********************************************************
//Receiving signals
//***********************************************************

void sendConfirm(){
    union sigval val;
    switch (mode)
    {
    case 0:
        if( kill(senderPid, SIGUSR1) != 0){
            perror("Error during resenging SIGUSR1");
            exit(1);
        }
        break;
    
    case 1:
        val.sival_int = recivedSignals - 1;
        if( sigqueue(senderPid, SIGUSR1, val) != 0){
            perror("Error during resenging SIGUSR1");
            exit(1);
        }
        break;
    
    case 2:
        if( kill(senderPid, SIGRTMIN) != 0){
            perror("Error during resenging SIGRTMIN");
            exit(1);
        }
        break;

    default:
        break;
    }

}

//assumtion that the first SIGUSR1 is from the sender and receiving it from different will be ignored
void sigCounter(int sig, siginfo_t * info, void * context){
    if(senderPid == -1){
        senderPid = info->si_pid;
        recivedSignals++;
        printf("Received %d signal \n", recivedSignals);
        sendConfirm();
    }
    else if(senderPid == info->si_pid){
        recivedSignals++;
        printf("Received %d signal \n", recivedSignals);
        sendConfirm();
    }
    else{
        printf("Ignoring signal from different sender!\n");
    }
}


void setUpReception(){
    struct sigaction act;

    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGINT);

    act.sa_flags = SA_SIGINFO;
    
    switch (mode)
    {
    case 0: //kill mode
        act.sa_sigaction = sigCounter;
        if (sigaction(SIGUSR1, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }
        break;

    case 1: //sigqueue mode
        act.sa_sigaction = sigCounter;
        if (sigaction(SIGUSR1, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }
        break;

    case 2: //real time mode
        act.sa_sigaction = sigCounter;
        if (sigaction(SIGRTMIN, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }

        break;
    default:
        printf("No such mode\n");
        exit(1);
        break;
    }
    
}

//*********************************************************
//Resending signals
//*********************************************************

void resendKill(int sig, siginfo_t * info, void * context){
    if(senderPid == -1){
        printf("Something wrong, sender's id is unknown\n");
        exit(1);
    }
    else if(senderPid != info->si_pid){
        printf("Ignoring SIGUSR2 not from sender!\n");
        return;
    }
    
    if(kill(senderPid, SIGUSR2) != 0){
        perror("Error during resending SIGUSR2");
        exit(1);
    }

    printf("Sender's pid: %d\nReceived signals count: %d\n", senderPid, recivedSignals);
    exit(0);
}


void resendQueue(int sig, siginfo_t * info, void * context){
    if(senderPid == -1){
        printf("Something wrong, sender's id is unknown\n");
        exit(1);
    }
    else if(senderPid != info->si_pid){
        printf("Ignoring SIGUSR2 not from sender!\n");
        return;
    }
    
    union sigval val;
    val.sival_int = recivedSignals;

    if(sigqueue(senderPid, SIGUSR2, val) != 0){
        perror("Error during resending SIGUSR2");
        exit(1);
    }

    printf("Sender's pid: %d\nReceived signals count: %d\n", senderPid, recivedSignals);
    exit(0);

}

void resendRT(int sig, siginfo_t *info, void *context){
    if(senderPid == -1){
        printf("Something wrong, sender's id is unknown\n");
        exit(1);
    }
    else if(senderPid != info->si_pid){
        printf("Ignoring RT signal not from sender!\n");
        return;
    }
    
    if(kill(senderPid, SIGRTMIN + 1) != 0){
        perror("Error during resending SIGRTMIN + 1");
        exit(1);
    }

    printf("Sender's pid: %d\nReceived signals count: %d\n", senderPid, recivedSignals);
    exit(0);
}

void setUpResend(){
    struct sigaction act;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGINT);
    act.sa_flags = SA_SIGINFO;

    switch (mode)
    {
    case 0: //resend using kill
        act.sa_sigaction = resendKill;
        if (sigaction(SIGUSR2, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }    
        break;

    case 1: //resend using sigqueue
        act.sa_sigaction = resendQueue;
        if (sigaction(SIGUSR2, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }
        break;
    
    case 2: //resend using real time signals
        act.sa_sigaction = resendRT;
        if (sigaction(SIGRTMIN + 1, &act, NULL) != 0){
            perror("Error during establishing action");
            exit(1);
        }
        break;

    default:
        printf("No such mode\n");
        exit(1);
        break;
    }
    
    
}


int main(int argc, char **argv){
    printf("Catcher's pid: %d\n", getpid());

    if (argc != 2){
        printf("Wrong number of arguments\n");
        return 1;
    }

    if(strcmp(argv[1], "kill") == 0){
        mode = 0;
        setUpReception();
        setUpResend();
    }
    else if (strcmp(argv[1], "sigqueue") == 0)
    {
        mode = 1;
        setUpReception();
        setUpResend();
    }
    else if (strcmp(argv[1], "RT") == 0)
    {
        mode = 2;
        setUpReception();
        setUpResend();
    }
    else
    {
        printf("Wrong argument!\n");
        return 1;
    }
    
    sigset_t toBlock;
    sigfillset(&toBlock);
    sigdelset(&toBlock, SIGUSR1);
    sigdelset(&toBlock, SIGUSR2);
    sigdelset(&toBlock, SIGINT);
    sigdelset(&toBlock, SIGRTMIN);
    sigdelset(&toBlock, SIGRTMIN + 1);
    
    if(sigprocmask(SIG_BLOCK, &toBlock, NULL) != 0){
        perror("Error during blocking signals sigprocmask");
        exit(1);
    }

    while (1)
    {
        pause();//wait for signals
    }
    
    return 0;
}