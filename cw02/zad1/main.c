#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void generateFile(char *fileName, int linesCount, size_t bytesNumber){
    
    //create or open file to write result 
    //content will be overwrite
    FILE *fp;
    fp = fopen(fileName, "w");
    
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
    int destHandle = open(dest, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

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

//low and high are limits of sorting, they are used as idices of lines in file, bytesNum is number of bytes in each line
int partitionSys(int fileDes, int low, int high, size_t bytesNum){
    char *pivot = (char *) calloc(bytesNum, sizeof(char));
    char *line = (char *) calloc (bytesNum, sizeof(char));
    int pivotOffest = high * bytesNum; // positon of last line pivot


    int i = low - 1;

    for (int j = low; j < high; j++){
        
        //set pivot position as last line and read it
        //this have to be repeated because we have to use only two records in memory
        lseek(fileDes, pivotOffest, SEEK_SET);
        
        if (read(fileDes, pivot, bytesNum) != bytesNum){
            perror("Error during sorting using system fuctions");
            exit(1);
        }  
      

        //read line which will be examined now
        lseek(fileDes, j*bytesNum, SEEK_SET);

        if (read(fileDes, line, bytesNum) != bytesNum){
            perror("Error during sorting using system fuctions");
            exit(1);
        }

        //sorting using strcmp function, the rules of sorting were a bit confusing so I decided to sort that way
        if (strcmp(pivot, line) > 0 ){
            

            i++;
            lseek(fileDes, i * bytesNum, SEEK_SET);
            
            //use pivot as a buffer to swap lines, it's just to meet task goal about having only two records in memory
            if (read(fileDes, pivot, bytesNum) != bytesNum){
                perror("Error during sorting using system fuctions");
                exit(1);
            }
              
            lseek(fileDes, i * bytesNum, SEEK_SET);
            
            if (write(fileDes, line, bytesNum) != bytesNum){
                perror("Error during sorting using system fuctions");
                exit(1);
            }


            lseek(fileDes, j * bytesNum, SEEK_SET);
            
            
            if (write(fileDes, pivot, bytesNum) != bytesNum){
                perror("Error during sorting using system fuctions");
                exit(1);
            }
            
        }
    }

    //set pivot to it's position and return this position as index of partition 
    i++;
    
    lseek(fileDes, pivotOffest, SEEK_SET);
        
    if (read(fileDes, pivot, bytesNum) != bytesNum){
        perror("Error during sorting using system fuctions");
        exit(1);
        }
    
    lseek(fileDes, i*bytesNum, SEEK_SET);
    
    if (read(fileDes, line, bytesNum) != bytesNum){
        perror("Error during sorting using system fuctions");
        exit(1);
        }

    lseek(fileDes, i*bytesNum, SEEK_SET);
    
    if (write(fileDes, pivot, bytesNum) != bytesNum){
        perror("Error during sorting using system fuctions");
        exit(1);
    }

    lseek(fileDes, pivotOffest, SEEK_SET);
    
    if (write(fileDes, line, bytesNum) != bytesNum){
        perror("Error during sorting using system fuctions");
        exit(1);
    }
        

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
    
    qsSys(fd, 0, linesCount - 1, bytesNum);

    close(fd);

    return 0;
}


int partitionLib(FILE *filePtr, int low, int high, size_t bytesNum){
    char *pivot = (char *) calloc(bytesNum, sizeof(char));
    char *line = (char *) calloc (bytesNum, sizeof(char));
    int pivotOffest = high *bytesNum;


    int i = low - 1;

    for (int j = low; j < high; j++){
        
        //set pivot position as last line and read it
        fseek(filePtr, pivotOffest, 0);
        
        if (fread(pivot, sizeof(char), bytesNum, filePtr) != bytesNum){
            perror("Error during sorting using library fuctions");
            exit(1);
        }  
      
        fseek(filePtr, j * bytesNum, 0);

        if (fread(line, sizeof(char), bytesNum, filePtr) != bytesNum){
            perror("Error during sorting using library fuctions");
            exit(1);
        }


        if (strcmp(pivot, line) > 0 ){
            

            i++;
            fseek(filePtr, i * bytesNum, 0);
            
            //use pivot as a buffer to swap lines, it's just to meet task goal
            if (fread(pivot, sizeof(char), bytesNum, filePtr) != bytesNum){
                perror("Error during sorting using library fuctions");
                exit(1);
            }
              
            fseek(filePtr, i * bytesNum, 0);
            
            
            if (fwrite(line, sizeof(char), bytesNum, filePtr) != bytesNum){
                perror("Error during sorting using library fuctions");
                exit(1);
            }

            fseek(filePtr, j*bytesNum, 0);
            
            
            if (fwrite(pivot, sizeof(char), bytesNum, filePtr) != bytesNum){
                perror("Error during sorting using library fuctions");
                exit(1);
            }
            
        }
    }
    
    i++;
    
    fseek(filePtr, pivotOffest, 0);
        
    if (fread(pivot, sizeof(char), bytesNum, filePtr) != bytesNum){
        perror("Error during sorting using library fuctions");
        exit(1);
        }
    
    fseek(filePtr, i*bytesNum, 0);
    
    if (fread(line, sizeof(char), bytesNum, filePtr) != bytesNum){
        perror("Error during sorting using library fuctions");
        exit(1);
        }

    fseek(filePtr, i*bytesNum, 0);
    
    if (fwrite(pivot, sizeof(char), bytesNum, filePtr) != bytesNum){
        perror("Error during sorting using library fuctions");
        exit(1);
    }

    fseek(filePtr, pivotOffest, 0);
    
    if (fwrite(line, sizeof(char), bytesNum, filePtr) != bytesNum){
        perror("Error during sorting using library fuctions");
        exit(1);
    }
        


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
    
    qsLib(fPtr, 0, linesCount - 1, bytesNum);

    fclose(fPtr);

    return 0;
}



int main(int argc, char** argv){
    generateFile("aa.txt", 300, 10);
    copySys("aa.txt", "b.txt", 300, 10);
    qsWrapperLib("aa.txt", 300, 10);


    return 0;
}   