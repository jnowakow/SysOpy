#include "mylib.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


int MAX = 100;

struct MainTable *createTable(int size){
    if (size <= 0){
        printf("Size of table must be positive integer!\n");
        return NULL;
    }
    struct MainTable *newTable;
    newTable = (struct MainTable*) calloc(1, sizeof(struct MainTable));
    
    newTable->blocks = (struct OperationsBlock**) calloc(size, sizeof(struct OperationsBlock *));
    newTable->lastItemIndex = -1;

    return newTable;
}

int createPair(char *input, struct filesToCompare *pair){
    char *dlim = strchr(input, ':');
    if (dlim == NULL){
        printf("Wrong input format during parsing: %s", input);
        return -1;
    }

    
    int firstLen = dlim - input;

    pair->file1 = (char *) calloc(firstLen + 1, sizeof(char));
    pair->file2 = (char *) calloc(strlen(dlim) + 1, sizeof(char));

    
    for (int i = 0 ; i < firstLen; i++){
        pair->file1[i] = input[i];
    }

    for (int i = firstLen + 1, j = 0; i < strlen(input); i++, j++ ){
        pair->file2[j] = input[i];
    }
    
    
    return 0;

}

struct filesSequence * defineFilesSequence(char **input, int n){

    struct filesSequence *seq = (struct filesSequence *) calloc(1, sizeof(struct filesSequence));
    seq->pairs = (struct filesToCompare *) calloc(n, sizeof(struct filesToCompare));
    seq->size = n;
    
    for (int i = 0; i < n; i++){
        
        if (createPair(input[i], &seq->pairs[i]) != 0){
            printf("Error during parsing files sequence");
            for (int j = 0; j < i; j++){
                free(seq->pairs[i].file1);
                free(seq->pairs[i].file2);
            }
            free(seq->pairs);
            free(seq);
            return NULL;
        }
    }

    return seq;
}



char *compareAndWrite(struct filesToCompare *pair){
    char *tmpFileName = calloc(strlen(pair->file1) - 4 + strlen(pair->file2), sizeof(char));
    int len;
    len = sizeof(tmpFileName);
    memset(tmpFileName, 0, len);

    for(int i = 0; i < strlen(pair->file1) - 4; i++){//to remove .txt
        tmpFileName[i] = pair->file1[i];
    }
    strcat(tmpFileName, pair->file2);

    
    char *command = calloc(MAX, sizeof(char));
    len = sizeof(command);
    memset(command, 0, len);
    strcpy(command, "diff ");
    strcat(command, pair->file1);
    strcat(command, " ");
    strcat(command, pair->file2);
    strcat(command, " > ");
    strcat(command, tmpFileName);

    system(command);

    return tmpFileName;
}

int insertOperationsBlock(struct MainTable *table, char *tmpFileName){
    FILE *fp = fopen(tmpFileName, "r");
    
    if(fp == NULL){
        printf( "Can't open file %s", tmpFileName);
        return -1;
    }

    struct OperationsBlock *newBlock = (struct OperationsBlock *) calloc(1, sizeof(struct OperationsBlock));

    char *line = NULL;
    size_t lineSize = 0;
    char *operation = NULL;
    
    while (getline(&line, &lineSize, fp) >= 0)
    {
        if(isdigit(line[0])){
            newBlock->size++;
            if (newBlock->size == 1){//first operation
                newBlock->operation = (char **) calloc(1, sizeof(char *));
                operation = (char *) calloc(strlen(line), sizeof(char));
                strcpy(operation, line);
                
            } else{
                newBlock->operation = (char **) realloc(newBlock->operation, newBlock->size);
                newBlock->operation[newBlock->size - 2] = (char *) calloc(strlen(operation), sizeof(char));
                
                strcpy(newBlock->operation[newBlock->size - 2], operation);

                operation = (char *) realloc(operation, strlen(line));
                strcpy(operation, line);
                
            }
        
        }else{
            operation = (char *) realloc(operation, strlen(operation) + strlen(line));
            strcat(operation, line);
        }
    }
    
    newBlock->operation[newBlock->size - 1] = (char *) calloc(strlen(operation), sizeof(char));
    strcpy(newBlock->operation[newBlock->size - 1], operation);
    
    

    fclose(fp);
    free(operation);
    free(line);
    char *command = calloc(MAX, sizeof(char));
    int len = sizeof(command);
    memset(command, 0, len);
    strcpy(command, "rm -f ");
    strcat(command, tmpFileName);
    system(command);

    table->lastItemIndex++;
    table->blocks[table->lastItemIndex] = newBlock;

    return table->lastItemIndex;
}


int operationsCount(struct MainTable *table, int blockIndex){
    if (table->blocks[blockIndex] == NULL){
        printf("Block doesn't exist\n");
        return -1;
    }
    return table->blocks[blockIndex]->size;
}

int deleteBlock(struct MainTable *table, int index){
    if(table->blocks[index] == NULL){
        printf("Block doesn't exist\n");
        return -1;
    }

    for(int i = 0; i < table->blocks[index]->size; i++){
        deleteOperation(table, index, i);
    }
    free(table->blocks[index]);
    table->blocks[index] = NULL;
    
    return 0;
}

int deleteOperation(struct MainTable *table, int blockIndex, int operationIndex){
    if (table->blocks[blockIndex]->operation[operationIndex] == NULL){
        printf("Operation doesn't exist!");
        return -1;
    }
    free(table->blocks[blockIndex]->operation[operationIndex]);
    table->blocks[blockIndex]->operation[operationIndex] = NULL;

    return 0;
}


