#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>



void checkIfBlocked(){
    sigset_t blocked;
    sigpending(&blocked);
            
    if(sigismember(&blocked, SIGUSR1) == 1){
        printf("Signal SIGUSR1 is pending\n");
    }
    else
    {
        printf("Signal SIGUSR1 isn't pending\n");
    }
}

void handler(int sig){
    printf("I got SIGUSR1: %d\n", sig);
}

int main(int argc, char **argv){

    if (argc != 2){
        printf("Wrong number of arguments!\n");
        return 1;
    }
    
    printf("Output from parent\n");
    
    if (strcmp(argv[1], "ignore") == 0)
    {
        //it is easier to show how signals are ignored using SIGINT than SIGUSR1 so I will use it
        signal(SIGINT, SIG_IGN);
        
        printf("Raising SIGINT\n");
        raise(SIGINT);
        printf("After SIGINT\n");
        
        
        pid_t pid = fork();
        if(pid == 0){
            printf("\nOutput from child\n");
            printf("Raising SIGINT\n");
            raise(SIGINT);
            printf("After SIGINT\n");

        }
        else{
            pid_t pid2 = fork();
            if(pid2 == 0){
                execl("./child", "./child", "ignore", NULL);
            }
        }

    }
        
    else if (strcmp(argv[1], "handler") == 0){
        signal(SIGUSR1, handler);
        raise(SIGUSR1);
        
        pid_t pid = fork();
        if(pid == 0){
            printf("\nOutput from chlid\n");
            raise(SIGUSR1);
        }
         else{
            pid_t pid2 = fork();
            if(pid2 == 0){
                execl("./child", "./child", "handler", NULL);
            }
        }    
    }

    else if(strcmp(argv[1], "mask") == 0){
            
        //set SIGUSR1 as blocked
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        if(sigprocmask(SIG_SETMASK, &set, NULL) != 0){
            perror("Can't set blocked mask");
            return 1;
        }  

        raise(SIGUSR1);
        checkIfBlocked();

        pid_t pid = fork();
        if(pid == 0){
            printf("\nOutput from chlid\n");
            
            printf("Checking signal raised in parent\n");

            checkIfBlocked();

            printf("\nChecking signal raised in current procces\n");
            raise(SIGUSR1);
            checkIfBlocked();
        }
        else{
            pid_t pid2 = fork();
            if(pid2 == 0){
                execl("./child", "./child", "mask", NULL);
            }
        }
    }
    
    else{
        printf("Wrong argument\n");
        return 1;
    }
    
    return 0;
}