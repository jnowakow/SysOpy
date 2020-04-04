#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
 
#define LEN 100 //maximal length of line
#define CMDS 10 //maximal number of commands in line
#define ARGS 10 //maximal length of arguments for command
 

//this function prepare command to be executed with execvp
//assumption that command won't have more than 10 argument
char **parseArguments(char *command){
    int count = 0;
    char **args = (char **) calloc(ARGS + 2, sizeof(char *)); //create 2 more pointers for command name and NULL pointer

    char delims[2] = {' ', '\n'}; 
    
    char *next;
    next = strtok(command, delims);

    while (next != NULL)
    {
        args[count] = next;
        next = strtok(NULL, delims);
        count++;
    }
    
    args[count] = NULL;

    return args;
}


void executeLine(char *line){
    int cmdIndx = 0;
    char **commands = (char **) calloc(CMDS, sizeof(char *));

    //parse the line to extract commands with arguments
    char *cmd;
    cmd = strtok(line, "|");
    
    while (cmd != NULL)
    {
        commands[cmdIndx] = cmd;
        cmd = strtok(NULL, "|");
        cmdIndx++;
    }
    
    int curFd[2];
    int prevFd[2];

    for (int i = 0; i < cmdIndx; i++){

        if(pipe(curFd) != 0){
            perror("Error during openig pipe");
            exit(1);
        }

        pid_t pid = fork();    
        if(pid == 0){
            char ** args = parseArguments(commands[i]);
            
            //set this process output to stdout
            if(i != cmdIndx -1){ //execpt last command
                close(curFd[0]);
                if(dup2(curFd[1], STDOUT_FILENO) == -1){
                    perror("Error during setting output");
                    exit(1);
                }
            }

            //read output from last process
            if(i > 0){
                close(prevFd[1]);
                if(dup2(prevFd[0], STDIN_FILENO) == -1){ 
                    perror("Error during setting input");
                    exit(1);
                }
            }

            if( execvp(args[0], args) == -1){
                printf("Error during executing command %s\n", args[0]);
                exit(1);
            }
        }
        else
        {   
            //pipe won't be used anymore
            if( i > 0){
                close(prevFd[0]);
                close(prevFd[1]);
            }

            //remeber descriptors of current pipe so the next child can use it 
            if(i !=  cmdIndx -1){
                prevFd[0] = curFd[0];
                prevFd[1] = curFd[1];
            }
        }
        
    }

    //close the last pipe
    close(curFd[0]);
    close(curFd[1]);
    int status;

    wait(&status);//wait till children processes end, wait is sufficient because reading and writing will block the processes
    
    if(WEXITSTATUS(status) != 0){
        exit(1);
    }

    exit(0);
}



 
 
int main (int argc, char **argv){
  if (argc != 2){
    printf("Wrong number of arguments!\nUsage is ./prog file\n");
  }
 
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL){
    printf("Can't open file\n");
    return 1;
  }
 
  char line[LEN];
  int counter = 0;
 
  while (fgets(line, LEN, fp)) {
      
      counter++;
      pid_t pid = fork();
      if(pid == 0){
        fclose(fp);
        executeLine(line);
        return 0;
      }
      else{
        int status;
        wait(&status);
        if (WEXITSTATUS(status) != 0)
        {
            printf("Error during executing %d line!\n", counter);
        }
        
      }
   }

   fclose(fp);
   

   return 0;
}