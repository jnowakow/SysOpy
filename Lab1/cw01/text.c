#include <stdio.h>
#include "mylib.h"
#include <string.h>
#include <stdlib.h>
int size = 100;

int main(int argc, char* argv[]){
    struct MainTable *table = createTable(1);
    char **input = (char **) calloc(1, sizeof(char *));
    input[0] = (char *) calloc(12, sizeof(char));

    strcpy(input[0], "a.txt:b.txt");
        
    struct filesSequence *seq = defineFilesSequence(input, 1);

    char *tmp = compareAndWrite(&(seq->pairs[0]));
    printf("inserted index: %d\n", insertOperationsBlock(table, tmp));
    printf("operations count: %d\n", operationsCount(table,0));
    
    for (int i = 0; i < table->blocks[0]->size; i++)
    {
        printf("%s\n", table->blocks[0]->operation[i]);
    }
    
    printf("deletion status: %d\n", deleteBlock(table,0));
    

    return 0;
}