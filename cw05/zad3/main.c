#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char **argv){
    if (argc != 8){
        printf("Specify fifo name\n");
        return 1;
    }

    if(mkfifo(argv[1], 0666) == -1){
        perror("Fifo error");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0){
        execl("./consumer", "./consumer", argv[1], argv[2], "10", NULL);
    }

    for(int i = 0; i < 5; i++){
        pid = fork();
        if(pid == 0){
            char num[2];
            sprintf(num, "%d", (i+1) * 2);
            execl("./producer", "./producer", argv[1], argv[i+3], num, NULL);
        }
    }


    return 0;
}