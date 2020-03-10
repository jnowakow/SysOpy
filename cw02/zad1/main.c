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
    
    srand(time(0));//seed generator

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

void swap(char **str1, char **str2){
    char *tmp = *str1;
    *str1 = *str2;
    *str2 = tmp;
}

/*
char *getLineSys(int fileHandle, int index, size_t bytesNum){
    char *line = calloc(bytesNum, sizeof(char));
    lseek(fileHandle, )
}


void QSSys(char *file, )
*/



int main(int argc, char** argv){
    generateFile("aa.txt", 3, 10);
    copyLib("aa.txt", "b.txt", 3, 10);


    return 0;
}   