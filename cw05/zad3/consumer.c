#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define MARGIN 10

int main(int argc, char **argv){
    if ( argc != 4){
        printf("Wrong number of arguments\n");
        return 1;
    }

    FILE *fifo = fopen(argv[1], "r");
    if(fifo == NULL){
        perror("Cannot open FIFO");
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if ( output == NULL)
    {
        perror("Cannot open output file");
        return 1;
    }

    int num = atoi(argv[3]);
    
    char buff[num+ MARGIN];

    while (fgets(buff, num + MARGIN, fifo))
    {   
        fputs(buff, output);
    }
    
    

    fclose(output);
    fclose(fifo);

    return 0;
}