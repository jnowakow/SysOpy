#include <stdio.h>
#include "mylib.h"
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>

int size = 100;

FILE *resultFile;

void error(char *message){
    printf("%s", message);
    exit(1);
}

double timeDifference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / CLOCKS_PER_SEC);
}

void writeResult(char *message, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("%s\n", message);
    printf("\tREAL_TIME: %fl\n", timeDifference(start,end));
    printf("\tUSER_TIME: %fl\n", timeDifference(t_start->tms_utime, t_end->tms_utime));
    printf("\tSYSTEM_TIME: %fl\n", timeDifference(t_start->tms_stime, t_end->tms_stime));
    
    fprintf(resultFile, "%s\n", message);
    fprintf(resultFile, "\tREAL_TIME: %fl\n", timeDifference(start, end));
    fprintf(resultFile, "\tUSER_TIME: %fl\n", timeDifference(t_start->tms_utime, t_end->tms_utime));
    fprintf(resultFile, "\tSYSTEM_TIME: %fl\n", timeDifference(t_start->tms_stime, t_end->tms_stime));
    
}

char *commands[3] = {"compare_pairs", "remove_block", "remove_operation"};

int checkCommand(char* command){
    for(int i = 0; i < 3; i++){
        if(strcmp(command, commands[i]) == 0){
            return i+1;
        }
    }
    return 0;
}


int main(int argc, char* argv[]){
    
    
    if (argc < 4 || strcmp(argv[1], "create_table") != 0){
        error("Program call ./main create_tabele tabele_size compare_pairs a.txt:b.txt ...\n");
    }

    resultFile = fopen("raport3b.txt", "w");
    if (resultFile == NULL){
        error("Cannot open result file");
    }
    
    //cast 
    int size = atoi(argv[2]);
    struct MainTable *table = createTable(size);
    
    if (strcmp(argv[3], "compare_pairs") != 0){
        free(table);
        error("Program call ./main create_tabele tabele_size compare_pairs a.txt:b.txt ...\n");   
    }

    struct tms *startTime = calloc(1, sizeof(struct tms *));
    struct tms *endTime = calloc(1, sizeof(struct tms *));
    clock_t start;
    clock_t end;

    char **input;
    int pairsCount = 0;
    int blockIndex = 0;
    int operationIndex = 0;
    struct filesSequence *seq;
    char **tmpfiles;
    
    
    for(int i = 3; i < argc; i++){
        int com = checkCommand(argv[i]);
        if (com == 1){
            pairsCount = 0;
            while (i + pairsCount + 1 < argc && checkCommand(argv[i + pairsCount + 1]) == 0)
            {
                pairsCount++;
            }
            input = (char **) calloc(pairsCount, sizeof(char *));

            for (int j = 0; j < pairsCount; j++) {
                input[j] = argv[i+j+1];
            }
            

            seq = defineFilesSequence(input, pairsCount);
            tmpfiles = (char **) calloc(pairsCount, sizeof(char *));
            
            for (int k = 0; k < pairsCount; k++){
                //measure diff time
                start = times(startTime);
                tmpfiles[k] = compareAndWrite(&(seq->pairs[k]));   
                end = times(endTime);
                writeResult("Diff command: ",start, end, startTime, endTime);
                
                //printf("%s\n", tmpfiles[j]);
                //measure saving time
                start = times(startTime);
                insertOperationsBlock(table,tmpfiles[k]);
                end = times(endTime);
                
                writeResult("Saving operations block: ", start, end, startTime, endTime);

            }
            
            free(tmpfiles);
            
            i += pairsCount;
        }
        else if(com == 2){
            i++;
            blockIndex = atoi(argv[i]);
            //measure time
            start = times(startTime);
            deleteBlock(table, blockIndex);
            end = times(endTime);
            writeResult("Deleting block: ",start, end, startTime, endTime);
            
        }
        else
        {
            i++;
            blockIndex = atoi(argv[i]);
            i++;
            operationIndex = atoi(argv[i]);
            //measure time;

            start = times(startTime);
            deleteOperation(table, blockIndex, operationIndex);
            end = times(endTime);
            writeResult("Deleting operation: ",start, end, startTime, endTime);
            
        }
    }
    
    free(table);
    fclose(resultFile);
    
    return 0;
}