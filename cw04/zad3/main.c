#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>



void handler(int sig, siginfo_t *info, void *context){
    if(info->si_code == SI_QUEUE){
        char *msg = (char *) info->si_value.sival_ptr;

        printf("I received \"%s\" message from %d with signal %d\n", msg, info->si_pid, info->si_signo);
        
        free(msg);
        
    }
    else{
        printf("signal wasn't send with sigqueue\n");
    }
}



int main(int argc, char **argv){

    struct sigaction act;
    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    if(sigaction(SIGUSR1, &act, NULL) != 0){
        perror("Can't establish signal handler");
        return 1;
    }


    union sigval val;
    char *msg = (char *) calloc(12, sizeof(char));
    strcpy(msg, "Hello world");
    val.sival_ptr = (void *) msg;

    if(sigqueue(getpid(), SIGUSR1, val) != 0){
        perror("Can't send signal");
        return 1;
    };
    
    sleep(2);

    if(kill(getpid(), SIGUSR1) != 0){
        perror("Can't send signal");
        return 1;
    };
    
    
    return 0;
}