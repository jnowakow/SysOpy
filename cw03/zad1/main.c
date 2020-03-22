#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>//realpath
#include <dirent.h>// stat functions
#include <time.h>
#include <sys/stat.h> //struct stat
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ftw.h>

static int max = 200; // maximal length of real path
int modeGlobal = 0;
int sgnGlobal = 0;
int nGlobal = -1;
int depthGlobal = INT_MAX - 1;

char *transateFileTypeNFTW(int type){
    switch (type)
    {
        case FTW_F: return "regular file";
        case FTW_D: return "directory";
        case FTW_SL: return "symbolic link";
        default: return "unknown";
    };
}

int displayInfoNFTW(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
    if (ftwbuf->level > depthGlobal + 1|| strcmp(fpath, ".") == 0){
        return 0;
    }

    if (tflag == FTW_D){
        
        char *absPath = (char *) calloc(max, sizeof(char));
        realpath(fpath, absPath);

        pid_t pid = fork();
        if (pid < 0){
            perror("Fork error");
            exit(1);
        }
        if (pid == 0){
            char * cmd = (char *) calloc(max, sizeof(char));
            strcpy(cmd, "ls -l ");
            strcat(cmd, fpath);

            printf("Pid: %d\n", getpid());
            printf("Path: %s\n", fpath);
            system(cmd);
            free(cmd);
            exit(0);            
        }
        else{
            wait(NULL);// just wait for end of child process
        }

        free(absPath);
    }    


    return 0;
}
 

void printUsage(){
    printf("usage is ./programName path [-maxdepth depth] \n");
}


int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Please specify directory to be searched\n");
        return 1;
    }

    // ./main path
    if (argc == 2){
        nftw(argv[1], displayInfoNFTW, 10, FTW_PHYS);
    }
    else if (argc == 4)
    {
        // ./main path -maxdepth n
        if(strcmp(argv[2], "-maxdepth") == 0){
            depthGlobal = atoi(argv[3]);
            
            nftw(argv[1], displayInfoNFTW, 10, FTW_PHYS);
            
        }
  
        //input error - no such option
        else
        {
            printf("There is no such option\n");
            printUsage();
            return 1;
        }
        
    
    }
    else
    {
        printUsage();
        return 1;
    }
    
    return 0;
}