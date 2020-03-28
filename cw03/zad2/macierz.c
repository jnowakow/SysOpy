#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 100


void preprocess(FILE *fp, int *row, int *col){
    int r = 0;
    int c = 0;
    char *buff = (char *) calloc(1, sizeof(char));
    while (fread(buff, sizeof(char), 1, fp) == 1 )
    {
        if(r == 0 && strcmp(buff, " ") == 0) 
            c++;
        
        if(strcmp(buff, "\n") == 0) 
            r++;

    }
    c++;
    
    (*row) = r;
    (*col) = c;
    fseek(fp, 0, SEEK_SET);
}
//size_t rowLen(FILE *fp)

char* getRow(FILE *fp, int index){
    char *row = (char *) calloc(1, sizeof(char));
    char *ch = (char *) calloc(1,sizeof(char));
    int r = 0;
    size_t size = 0;
    //printf("row: %d\n", index);

    while (fread(ch, sizeof(char), 1, fp) == 1 )
    {
        //printf("%s \n", ch);
        if(strcmp(ch, "\n") == 0)
            r++;

        if(r == index){
            if(index == 0){
                strcpy(row,ch);
                size++;
            }
            while (fread(ch, sizeof(char), 1, fp) == 1)
            {   
                if(size == 0 && index != 0){
                    strcpy(row,ch);
                    size++;
                }
                else if(strcmp(ch,"\n") == 0 ){
                    r++;
                    break;
                }
                else
                {   
                    size++;
                    row = (char *) realloc(row, size);
                    strcat(row, ch);
                }
            }
            
        }
        
    }
    //printf("%s", row);
    size++;
    row = (char *) realloc(row, size);
    strcat(row, "\n");
    fseek(fp, 0, SEEK_SET);
    return row;
}

char *rowToCol(char *row, int r, int c, size_t *s){
    size_t size = strlen(row);
    char *col = (char *) calloc(size, sizeof(char));
    //printf("%s\n", row);
    for (int i = 0; i < strlen(row); i++){
        //printf("%ch", row[i]);
        if(row[i] == 44){ //ascii code for ,
            col[i] = '\n';
        }
        else if( row[i] == 10) //ascii code for \n
        {
             continue;
        }
        else
        {
            col[i] = row[i];
        }
    }
    (*s) = size;
    return col;
}

FILE *transposed(FILE *fp, int row, int col){
    FILE *tr = fopen("transposed", "w");
   
    
    char *line;
    char *column;
    char **tmpNames = (char **) calloc(row, sizeof(char *));
    size_t size;
    
    

    for(int i = 0; i < row; i++){
        line = getRow(fp, i);
        //printf("%s", line);
        column = rowToCol(line, row, col, &size);
        tmpNames[i] = (char *) calloc(4, sizeof(char)); //assuming there won't be more then 9 rows
        sprintf(tmpNames[i],"col%d",i);

        FILE *tmp = fopen(tmpNames[i], "w");

        fwrite(column, sizeof(char), size, tmp);
        fclose(tmp);
        free(line);
        free(column);           
    }

    char **list = (char **) calloc(row + 4, sizeof(char *));
    list[0] = "/usr/bin/paste";
    list[1] = "-d";
    list[2] = ",";
    
    int len = 0;
    for (int i = 0; i < row; i++)
    {
        len = snprintf(NULL, 0, "/tmp/%d", i)+1;
        list[i+3] = calloc(len, sizeof(char));
        snprintf(list[i+3], len, "%s", tmpNames[i]);
    }
    
    list[row + 3] = NULL;
    pid_t pid = fork();
    if(pid == 0){
        dup2(fileno(tr), 1);
        execv(list[0], list);
        exit(0);
    }
    else
    {
        wait(NULL);
    }
    
    
    for (int i = 0 ; i < row; i++){
        free(tmpNames[i]);
    }

    free(tmpNames);



    char *cmd = (char *) calloc(N, sizeof(char));
    strcpy(cmd, "rm -f col*");
    
    system(cmd);
    free(cmd);
    
    return tr;

}

/*
char *getCol(FILE *fp, int index){
    char *col = (char *) calloc(1, sizeof(char));
    char *ch = (char *) calloc(1,sizeof(char));
    int c = 0;
    size_t size = 0;
    //printf("row: %d\n", index);

    while (fread(ch, sizeof(char), 1, fp) == 1 )
    {
        //printf("%s \n", ch);
        if(strcmp(ch,",") == 0)
            c++;

        if(strcmp(ch, "\n") == 0)
            c = 0;

        if(c == index){
            if(size == 0){
                size++;
                strcpy(col,ch);
                printf("%s ", ch);
            }
            else{
                size++;
                col = (char *) realloc(col, size);
                strcat(col, ch);
                
                while (fread(ch, sizeof(char), 1, fp) == 1)
                {
                    if(strcmp(ch,",") == 0 || strcmp(ch,"\n") == 0 ){
                        c++;
                        size++;
                        col = (char *) realloc(col, size);
                        strcat(col, ",");
                        break;
                    }

                    size++;
                    col = (char *) realloc(col, size);
                    strcat(col, ch);
                }
            }
        }
        
    }
    printf("\n col: %s\n", col);
    //printf("%s", row);
    fseek(fp, 0, SEEK_SET);
    return col;

}
*/


/*
void openFiles(char *list, char*mat_A, char ** mat_B, char **mat_C){
    printf("lala\n");
    char ch;
    char *name = (char *) calloc(1, sizeof(char)); 
    int size = 0;
    int i = 0;
    FILE *fp = fopen(list, "r");
    int len;

    while (fread(&ch, sizeof(char), 1, fp ) == 1)
    {
        if (ch == 32 || ch == 10){ //ascii code for space and new line
            switch (i)
            {
            case 0:
                printf("da %s\n", name);
                len = snprintf(NULL, 0, "%s", name) + 1;
                printf("what's going on\n");
                (*mat_A) =  calloc(len,sizeof(char));
                printf("adfa\n");
                strcpy(&(*mat_A), name);
                printf("%s\n", &(*mat_A));
                size = 0;
                name = (char *) realloc(name, 1);
                break;
            case 1:
                printf("fda, %s\n", name);
                len = snprintf(NULL, 0, "%s", name) + 1;
                printf("what's going on\n");
                (*mat_B) =  calloc(len,sizeof(char));
                strcpy(&(*mat_B), name);
                size = 0;
                name = (char *) realloc(name, 1);
                
                break;
            case 2:
                printf("sysopy %s\n", name);
                len = snprintf(NULL, 0, "%s", name) + 1;
                printf("what's going on\n");
                (*mat_C) =  calloc(len,sizeof(char));
                
                strcpy(&(*mat_C), name);
                printf("wrr\n");
                //size = 0;
                //name = (char *) realloc(name, 1);

                break;
            default:
                break;
            }
            i++;
        }
        else if( size == 0){
            printf("maby here?\n");
            strcpy(name, &ch);
            size++;
        }
        else
        {
            printf("or here?\n");
            size++;
            name = (char *) realloc(name, size);
            strcat(name, &ch);
            printf("%s\n", name);
        }
        

    }
    
}
*/

/*
char *dot(char *A, char *B){
    
    while ()
    {
         code 
    }
    

}
*/


void mulBlock(FILE *mat_A, FILE *tr_mat_B, int row, int col, int from, int to){
    char *row_A;
    char *row_B;

    
    for(int i = from; i <= to; i++){
        row_B = getRow(tr_mat_B, i);

        //open file for column

        for(int j = 0; j < row; j++){
            row_A = getRow(mat_A, j);

            //wrtie dot product to file
            //dot();

        }
    }
}


int main(int argc, char **argv){
    
    /*char *nameA = NULL;
    char *nameB = NULL;
    char *nameC = NULL;

    openFiles(argv[1], &nameA, &nameB, &nameC);
    */

    int row_A, col_A;
    //printf("%s %ld\n", nameA, strlen(nameA));
    FILE *mat_A = fopen(argv[1], "r");
    if (mat_A == NULL){
        perror("Can't open matrix A");
        return 1;
    }
    
    int row_B, col_B;
    FILE *mat_B = fopen(argv[2], "r"); 
    if(mat_B == NULL){
        perror("Can't open matrix B");
        return 1;
    }

    FILE *result = fopen(argv[3], "a");
    if(result == NULL){
        perror("Can't open matrix C");
        return 1;
    }

    

    preprocess(mat_A, &row_A, &col_A);
    preprocess(mat_B, &row_B, &col_B);
  

    FILE *tr = transposed(mat_B, row_B, col_B);

    int width = col_B / atoi(argv[4]);
    int flag; //information for number of rows for last process
    if (col_B %  atoi(argv[4]) == 0)
        flag = 1;
    else
        flag = 0;
    

    




    fclose(tr);
    fclose(mat_A);
    fclose(mat_B);
    fclose(result);


    return 0;
}