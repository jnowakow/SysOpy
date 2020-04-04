#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MARGIN 10


int main(int argc, char **argv){
    if ( argc != 4){
        printf("Wrong number of arguments\n");
        return 1;
    }

    
    int fifo = open(argv[1], O_WRONLY);
    if (fifo == -1){
        perror("Cannot open fifo");
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

    srand(time(NULL));

    while (fgets(buff, num + MARGIN, input))
    {   
        sprintf(output, "#%d#%s", getpid(), buff);
        write(fifo, output, strlen(output)*sizeof(char));
        sleep(1 + rand() % 2);
    }
    
    

    fclose(input);
    close(fifo);

    return 0;
}