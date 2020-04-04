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

    FILE *fifo = fopen(argv[1], "w");
    if(fifo == NULL){
        perror("Cannot open FIFO");
        return 1;
    }

    FILE *input = fopen(argv[2], "r");
    if ( input == NULL)
    {
        perror("Cannot open input file");
        return 1;
    }

    int num = atoi(argv[3]);
    
    char buff[num + MARGIN];
    char output[num + MARGIN];

    while (fgets(buff, num + MARGIN, input))
    {   
        sprintf(output, "#%d#%s", getpid(), buff);
        fputs(output, fifo);
        sleep(1);
    }
    
    

    fclose(input);
    fclose(fifo);

    return 0;
}