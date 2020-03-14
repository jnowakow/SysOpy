#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
FILE *resultFile; // wyniki.txt - file containing measured times 


void generateFile(char *fileName, int linesCount, size_t bytesNumber){
    
    //create or open file to write result 
    //content will be overwrite
    FILE *fp;
    fp = fopen(fileName, "w");
    
    if (fp == NULL ){
        perror("Cannot open or create file to save generated records");
        exit(1);
    }

    srand(time(0));

    char *line;
    line = (char *) calloc(bytesNumber, sizeof(char));

    for (int i = 0; i < linesCount; i++){
        for (int j = 0; j < bytesNumber - 1; j++){
            int randNum = rand() % (int) (sizeof(charset) - 1);
            line[j] = charset[randNum];
        }
        line[bytesNumber-1] = '\n';
        
        fwrite(line, sizeof(char), bytesNumber, fp);
        memset(line, 0, bytesNumber);

    }

    free(line);
    fclose(fp);
}

void copySys(char *source, char* dest, int linesCount, size_t bytesNum){

    int srcHandle = open(source, O_RDONLY);
    int destHandle = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (srcHandle == -1){
        perror("Cannot open source file");
        exit(1);
    }

    if (destHandle == -1){
        perror("Cannot open or create result file");
        exit(1);
    }

    char *buff;
    buff = (char *) calloc(bytesNum, sizeof(char));

    while (read(srcHandle, buff, bytesNum) > 0)
    {
        write(destHandle, buff, bytesNum);
    }
    

    free(buff);
    close(srcHandle);
    close(destHandle);

}

void copyLib(char *source, char* dest, int linesCount, size_t bytesNum){
    FILE *srcPtr = fopen(source, "r");
    FILE *destPtr = fopen(dest, "w");

    if (srcPtr == NULL ){
        perror("Cannot open source file");
        exit(1);
    }
    if (destPtr == NULL){
        perror("Cannot open or create result file");
        exit(1);
    }

    char *buff;
    buff = (char *) calloc(bytesNum, sizeof(char));

    while (fread(buff, sizeof(char), bytesNum, srcPtr) > 0)
    {
        fwrite(buff, sizeof(char), bytesNum, destPtr);
    }
    

    free(buff);
    fclose(srcPtr);
    fclose(destPtr);
}


//read line with specified index to buff
void readLineSys(int fileDes, char *buff, int index, size_t bytesNum){
        lseek(fileDes, index * bytesNum, SEEK_SET);
        
        if (read(fileDes, buff, bytesNum) != bytesNum){
            perror("Error during sorting using system fuctions");
            exit(1);
        }  
    

}
//write line from buff to line in file with specified index
void writeLineSys(int fileDes, char *buff, int index, size_t bytesNum){
    lseek(fileDes, index * bytesNum, SEEK_SET);
            
    if (write(fileDes, buff, bytesNum) != bytesNum){
        perror("Error during sorting using system fuctions");
            exit(1);
    }
}



//low and high are limits of sorting, they are used as idices of lines in file, bytesNum is number of bytes in each line
int partitionSys(int fileDes, int low, int high, size_t bytesNum){
    char *pivot = (char *) calloc(bytesNum, sizeof(char));
    char *line = (char *) calloc (bytesNum, sizeof(char));
    

    int i = low - 1;

    for (int j = low; j < high; j++){
        
        //set pivot position as last line and read it
        //this have to be repeated because we have to use only two records in memory
        readLineSys(fileDes, pivot, high, bytesNum);

        //read line which will be examined now
        readLineSys(fileDes,line, j, bytesNum);
        
            //sorting using strcmp function, the rules of sorting were a bit confusing so I decided to sort that way
        if (strcmp(pivot, line) > 0 ){
            

            i++;
            readLineSys(fileDes, pivot, i, bytesNum);
        
            writeLineSys(fileDes, line, i, bytesNum);

            writeLineSys(fileDes, pivot, j, bytesNum);
            
        }
    }

    //set pivot to it's position and return this position as index of partition 
    i++;
    
    readLineSys(fileDes, pivot, high, bytesNum);
    readLineSys(fileDes, line, i, bytesNum);
    
    writeLineSys(fileDes, pivot, i, bytesNum);
    writeLineSys(fileDes, line, high, bytesNum);
        

    //free memory
    free(pivot);
    free(line);

    return i;
}

void qsSys(int fileDes, int low, int high, size_t bytesNum){
    if (low < high){
        int part = partitionSys(fileDes, low, high, bytesNum);
        
        qsSys(fileDes, low, part -1, bytesNum);
        qsSys(fileDes, part +1, high, bytesNum);
    }
}

//it's used to open the file just once
int qsWrapperSys(char *file, int linesCount, size_t bytesNum){
    int fd = open(file, O_RDWR);
    
    if (fd == -1){
        perror("Cannot open file to sort");
        exit(1);
    }

    qsSys(fd, 0, linesCount - 1, bytesNum);

    close(fd);

    return 0;
}

void readLineLib(FILE *filePtr, char * buff, int index, size_t bytesNum){
    fseek(filePtr, index * bytesNum, 0);
        
        if (fread(buff, sizeof(char), bytesNum, filePtr) != bytesNum){
            perror("Error during sorting using library fuctions");
            exit(1);
        }  

}

void writeLineLib(FILE *filePtr, char *buff, int index, size_t bytesNum){
    fseek(filePtr, index * bytesNum, 0);
                 
    if (fwrite(buff, sizeof(char), bytesNum, filePtr) != bytesNum){
        perror("Error during sorting using library fuctions");
        exit(1);
    }

}


int partitionLib(FILE *filePtr, int low, int high, size_t bytesNum){
    char *pivot = (char *) calloc(bytesNum, sizeof(char));
    char *line = (char *) calloc (bytesNum, sizeof(char));
    

    int i = low - 1;

    for (int j = low; j < high; j++){
        
        //set pivot position as last line and read it
        readLineLib(filePtr, pivot, high, bytesNum);
        
        readLineLib(filePtr, line, j, bytesNum);


        if (strcmp(pivot, line) > 0 ){
            i++;
            
            //use pivot as a buffer to swap lines, it's just to meet task goal
            readLineLib(filePtr, pivot, i, bytesNum);

            //swap lines in the file
            writeLineLib(filePtr, line, i, bytesNum);  
            writeLineLib(filePtr, pivot, j, bytesNum);
            
        }
    }
    
    i++;
    
    //write pivot in the right position
    readLineLib(filePtr, pivot, high, bytesNum);
    readLineLib(filePtr, line, i, bytesNum);
    
    writeLineLib(filePtr, pivot, i, bytesNum);
    writeLineLib(filePtr, line, high, bytesNum);
        
    free(pivot);
    free(line);
    return i;
}

void qsLib(FILE *fPtr, int low, int high, size_t bytesNum){
    if (low < high){
        int part = partitionLib(fPtr, low, high, bytesNum);
        
        qsLib(fPtr, low, part -1, bytesNum);
        qsLib(fPtr, part +1, high, bytesNum);
    }
}


int qsWrapperLib(char *file, int linesCount, size_t bytesNum){
    FILE *fPtr = fopen(file, "r+");
    if (fPtr == NULL){
        perror("Cannot open file to sort");
        exit(1);
    }
    qsLib(fPtr, 0, linesCount - 1, bytesNum);

    fclose(fPtr);

    return 0;
}


double timeDiff(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void writeResult(struct tms *start,struct tms *end){
    fprintf(resultFile,"\tUSER_TIME: %fl\n", timeDiff(start->tms_utime,end->tms_utime));
    fprintf(resultFile,"\tSYSTEM_TIME: %fl\n\n", timeDiff(start->tms_stime,end->tms_stime));
}


int main(int argc, char** argv){
    struct tms *startTms = (struct tms*) calloc(1, sizeof(struct tms));
    struct tms *endTms = (struct tms*) calloc(1, sizeof(struct tms));


    resultFile = fopen("wyniki.txt", "a");
    if(resultFile == NULL){
        perror("Cannot open file to save times");
        exit(1);
    }

    if (strcmp(argv[1], "generate") == 0){
        if (argc < 5){
            printf("Wrong number of arguments!\n");
            fclose(resultFile);
            return 1;
        }
        int linesCount = (int) strtol(argv[3], NULL, 10);
        int bytesNum = (int) strtol(argv[4], NULL, 10);
        bytesNum++;//to make place for endline character
        generateFile(argv[2],linesCount, bytesNum);
    }
    else if (strcmp(argv[1], "copy") == 0){
        if (argc < 7){
            printf("Wrong number of arguments!\n");
            fclose(resultFile);
            return 1;
        }
        int linesCount = (int) strtol(argv[4], NULL, 10);
        int bytesNum = (int) strtol(argv[5], NULL, 10);
        bytesNum++;//to make place for endline character
        if (strcmp(argv[6], "lib") == 0){
            times(startTms);
            copyLib(argv[2], argv[3], linesCount, bytesNum);
            times(endTms);
            fprintf(resultFile, "Copy in mode lib file having %d lines %d bytes each\n", linesCount, bytesNum - 1);
            writeResult(startTms, endTms);
        }
        else if (strcmp(argv[6], "sys") == 0){
            times(startTms);
            copySys(argv[2], argv[3], linesCount, bytesNum);
            times(endTms);
            fprintf(resultFile, "Copy in mode sys file having %d lines %d bytes each\n", linesCount, bytesNum - 1);
            writeResult(startTms, endTms);
        }
        else
        {
            printf("No such copy mode!\n");
            fclose(resultFile);
            return 1;
        }
    }
    else if (strcmp(argv[1], "sort") == 0){
        if (argc < 6){
            printf("Wrong number of arguments!\n");
            fclose(resultFile); 
            return 1;
        }
        int linesCount = (int) strtol(argv[3], NULL, 10);
        int bytesNum = (int) strtol(argv[4], NULL, 10);
        bytesNum++;//to make place for endline character
        
        if (strcmp(argv[5], "lib") == 0){
            times(startTms);
            qsWrapperLib(argv[2], linesCount, bytesNum);
            times(endTms);
            fprintf(resultFile, "Sort in mode lib file having %d lines %d bytes each\n", linesCount, bytesNum - 1);
            writeResult(startTms, endTms);
        }
        else if (strcmp(argv[5], "sys") == 0){
            times(startTms);
            qsWrapperSys(argv[2], linesCount, bytesNum);
            times(endTms);
            fprintf(resultFile, "Sort in mode sys file having %d lines %d bytes each\n", linesCount, bytesNum - 1);
            writeResult(startTms, endTms);
        }
        else
        {
            printf("No such sorting mode!\n");
            fclose(resultFile);    
            return 1;
        }
    }
    else
    {
        printf("This program can only generate, copy, and sort files\n");
        fclose(resultFile);    
        return 1;
    }

    fclose(resultFile);    

    return 0;
}   