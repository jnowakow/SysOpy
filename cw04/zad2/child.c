#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int main(int argc, char **argv){

    printf("\nOutput from exec process:\n");
    if (strcmp(argv[1], "ignore") == 0){
        printf("Raising SIGINT\n");
        raise(SIGINT);
        printf("After SIGINT\n");
    }

    else if (strcmp(argv[1], "mask") == 0)
    {
        raise(SIGUSR1);

        sigset_t set;
        sigpending(&set);
        
        if(sigismember(&set, SIGUSR1) == 1){
            printf("Signal SIGUSR1 is blocked\n");
        }
        else
        {
            printf("Signal SIGUSR1 isn't blocked\n");
        }
        
    }
    else if (strcmp(argv[1], "handler") == 0){
        raise(SIGUSR1);
    }
    else
    {
        printf("wrong argument!\n");
        return 1;
    }
    
    return 0;
}