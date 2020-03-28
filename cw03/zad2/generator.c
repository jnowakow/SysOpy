#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 100


void fillMatrix(FILE *fp, int row, int col){
    //printf("wrr\n");
    int n;
    int size;
    char *num = (char *) calloc(5, sizeof(char));

    for(int i = 0; i < row; i++){
        n = (rand() % 201) - 100 ;
        sprintf(num, "%d ", n);
        
        size = strlen(num);
        char *line = (char *) calloc(size, sizeof(char));
        strcpy(line, num);
        for(int j = 1; j < col - 1; j++){
            n = (rand() % 201) - 100;
            sprintf(num, "%d ", n);
            size += strlen(num);
            line = (char *) realloc(line, size);
            strcat(line, num);
        }
        n = (rand() % 201) - 100;
        sprintf(num, "%d", n);
        size += (strlen(num) + 1);
        line = (char *) realloc(line, size);
        strcat(line, num);
        strcat(line, "\n");
        memset(num, 0, 5);
        fwrite(line, sizeof(char), size, fp);
        free(line);
    }

    free(num);
}


void generatePair(int num, int min, int max){
    char *A_name = (char *) calloc(N, sizeof(char));
    char *B_name = (char *) calloc(N, sizeof(char));
    
    sprintf(A_name, "A%d.txt", num);
    sprintf(B_name, "B%d.txt", num);
    
    FILE *A = fopen(A_name, "w");
    FILE *B = fopen(B_name, "w");
    
    int row_A, col_A, row_B, col_B;
   
    row_A = (rand() % (max - min))  + min + 1 ;
    col_A = (rand() % (max- min)) + min + 1 ;
    row_B = col_A;
    col_B = (rand() % (max - min)) + min + 1 ;

    fillMatrix(A, row_A, col_A);
    fillMatrix(B, row_B, col_B);


    fclose(A);
    fclose(B);
    free(A_name);
    free(B_name);
}

int main(int argc, char **argv){
    srand(time(0));
    
    int pairs = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);

    for(int i = 0; i < pairs; i++){
        generatePair(i,min, max);
    }

    return 0;
}
