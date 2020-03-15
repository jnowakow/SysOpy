#include <stdio.h>
#include <stdlib.h>
#include <limits.h>//realpath
#include <dirent.h>// stat functions
#include <time.h>
#include <sys/stat.h> //struct stat
#include <string.h>

static int max = 200; // maximal length of real path

char *transateFileType(unsigned char type){
    switch (type)
    {
    case DT_BLK:
        return "block device";
        break;
    
    case DT_CHR:
        return "character device";
        break;
    
    case DT_DIR:
        return "directory";
        break;
    
    case DT_FIFO:
        return "named pipe (FIFO)";
        break;
    
    case DT_LNK:
        return "symbolic link";
        break;

    case DT_REG:
        return "regular file";
        break;
    
    case DT_SOCK:
        return "socekt";
        break;
    
    case DT_UNKNOWN:
        return "file could not be determined";
        break;
    default:
        return "";
        break;
    }
}

//get acutall date and comapre check if the given one meets the time constraint
int checkTime(struct tm *checkedTime, char *sgn, int n ){
    time_t currentTime;
    struct tm *currTime;
    currentTime = time(NULL);
    currTime = localtime(&currentTime);

    //dates represented as sum of days
    int curr = 0;
    int checked = 0;
    curr = currTime->tm_mday + 31*currTime->tm_mon + 365*currTime->tm_year;
    checked = checkedTime->tm_mday + 31*checkedTime->tm_mon + 365*checkedTime->tm_year;

    if (sgn == NULL){
        if (curr - checked == n){
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if(strcmp(sgn, "+") == 0)
    {
        if(curr - checked > n){
            return 1;
        }
        else
        {
            return 0;
        }
    }
     else if(strcmp(sgn, "-") == 0)
    {
        if(curr - checked < n){
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void printInfo(char *absPath, struct stat *stats, unsigned char type, struct tm *modTime, struct tm *accTime ){
    char *dtsc = (char *) calloc(40, sizeof(char));//date to string converter
    
    printf("Path:   %s\n", absPath);
            printf("Links count:    %ld\n", stats->st_nlink);
            printf("Type:   %s\n", transateFileType(type));
            printf("Size:   %ld\n", stats->st_size);
            if (strftime(dtsc, 40, "%d.%m.%Y", modTime) == 0){
                printf("Error during convering date to string!\n");
            }
            printf("Modification time:  %s\n", dtsc);
            if (strftime(dtsc, 40, "%d.%m.%Y", accTime) == 0){
                printf("Error during convering date to string!\n");
            }
            printf("Access time:    %s\n\n", dtsc);

    free(dtsc);
}


//mode 0 - atime or mtime wasn't used
//mode 1 - atime used
//mode 2 - mtime used
//sgn + or - in mtime or atime option or NULL if wasn't specified 
//n argument of atime or mtime call, means time of access or modification in n*24h period, -1 if nor mtime or atime was used
void searchUsingStat(char *dirPath, int depth, int mode, char *sgn, int n){
    if(depth < 0){
        return;
    }

    DIR *dir = opendir(dirPath);

    if(dir == NULL){
        perror("Cannot open directory with opendir");
        exit(1);
    }

    struct dirent *fptr;
    struct tm *modTime;
    struct tm *accTime;
    char *absPath = (char *) calloc(max, sizeof(char));
    char *nextPath = (char *) calloc(max, sizeof(char));
    struct stat stats;

    while ((fptr = readdir(dir)) != NULL)
    {
        if (strcmp(fptr->d_name, ".") == 0 || strcmp(fptr->d_name, "..") == 0){
            continue;
        }

        strcpy(nextPath, dirPath);
        strcat(nextPath, "/");
        strcat(nextPath, fptr->d_name);
        realpath(nextPath, absPath);

        if(lstat(nextPath, &stats) < 0){
            perror("lstat error");
            exit(1);
        }

        modTime = localtime(&stats.st_mtime);
        accTime = localtime(&stats.st_atime);

        switch (mode)
        {
        case 0:
            printInfo(absPath,&stats,fptr->d_type,modTime, accTime);
            break;
        
        case 1:
            if (checkTime(accTime, sgn, n) > 0){
                printInfo(absPath,&stats,fptr->d_type,modTime, accTime);
            }
            break;
        
        case 2:
            if (checkTime(modTime, sgn, n) > 0){
                printInfo(absPath,&stats,fptr->d_type,modTime, accTime);
            }

        default:
            break;
        }

        if(fptr->d_type == DT_DIR){
            searchUsingStat(nextPath, depth -1, mode, sgn, n) ;
        }

    }

    free(absPath);
    free(nextPath);
    free(fptr);
    closedir(dir);
}




int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Please specify directory to be searched\n");
        return 1;
    }
    
    searchUsingStat(".", INT_MAX, 1, "-", 1);

    return 0;
}