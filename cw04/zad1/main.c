#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>


int running = 1; //boolean flag to check if program is running or waitng for signal


void listDir(char *dirPath){
   
    DIR *dir = opendir(dirPath);

    if(dir == NULL){
        perror("Cannot open directory with opendir");
        exit(1);
    }

    struct dirent *fptr;
    
    while ((fptr = readdir(dir)) != NULL)
    {
        if (strcmp(fptr->d_name, ".") == 0 || strcmp(fptr->d_name, "..") == 0){
            continue;
        }

        printf("%s\n", fptr->d_name);       

    }

    free(fptr);
    closedir(dir);
}



void sigactionHandler(int sig){
    if(running == 0){
        running = 1;
    }
    else
    {  
        running = 0;
        printf("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu\n");

        //set temporary mask so the process will wait only for SIGINT and SIGTSTP
        sigset_t set;
        sigfillset(&set);
        sigdelset(&set, SIGINT);
        sigdelset(&set, SIGTSTP);
        
        //wait for SIGINT or SIGTSTP
        sigsuspend(&set);
    }

}

void sigintHandler(int sig){
    printf("\nOdebrano sygnał SIGINT: %d\n", sig);
    exit(0);
}


int main(int argc, char **argv){

    //set SIGINT handling
    signal(SIGINT, sigintHandler);

    //set SIGSTP handling
    struct sigaction act; 
    act.sa_handler = sigactionHandler; 
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0; 

    sigaction(SIGTSTP, &act, NULL);
    
    
    while (1)
    {
        listDir(getcwd(NULL, 0));
        sleep(1);
    }
    
    return 0;
}