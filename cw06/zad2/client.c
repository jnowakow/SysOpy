#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "header.h"

int myId = -1;
int acitvated = FALSE;
mqd_t serverQueue = -1;
mqd_t clientQueue = -1;
mqd_t otherQueue = -1;

void removeQueue()
{
    if (otherQueue != -1)
    {
        if (mq_close(otherQueue) == -1)
        {
            perror("Can't close other client queue");
            exit(1);
        }
    }

    if (mq_close(serverQueue) == -1)
    {
        perror("Can't close server queue");
        exit(1);
    }

    if (mq_close(clientQueue) == -1)
    {
        perror("Can't close queue");
        exit(1);
    }

    char name[15];

    sprintf(name, "%s%d", CLIENT_BASE, getpid());

    if (mq_unlink(name) == -1)
    {
        perror("Can't remove the queue");
        exit(1);
    }
}

void init(char *clientName)
{

    serverQueue = mq_open(SERVER_NAME, O_WRONLY);
    if (serverQueue == -1)
    {
        perror("Can't open server queue");
        exit(1);
    }
    //printf("file desc: %d\n", serverQueue);
    char msg[MAX_MSG_SIZE];
    sprintf(msg, "I %s", clientName);

    if (mq_send(serverQueue, msg, MAX_MSG_SIZE, INIT) == -1)
    {
        printf("can't send message\n");
        exit(1);
    }

}

//read commands from stdin
void run()
{
    char *cmd = (char *)calloc(MAX_MSG_SIZE, sizeof(char));

    fgets(cmd, MAX_MSG_SIZE, stdin);
    cmd = strtok(cmd, "\n");
    if (strcmp(cmd, "STOP") == 0)
    {
        char msg[MAX_MSG_SIZE];
        sprintf(msg, "S %d", myId);

        //send msg to server
        if (mq_send(serverQueue, msg, MAX_MSG_SIZE, STOP) == -1)
        {
            perror("Can't send stop");
            exit(1);
        }
        //end work

        exit(0);
    }
    else if (strcmp(cmd, "DISCONNECT") == 0)
    {
        char msg[MAX_MSG_SIZE];
        sprintf(msg, "D %d", myId);

        //send msg to server
        if (mq_send(serverQueue, msg, MAX_MSG_SIZE, DISCONNECT) == -1)
        {
            printf("can't send message\n");
            exit(1);
        }

        if (mq_close(otherQueue) == -1)
        {
            perror("can't close other queue");
            exit(1);
        }
        otherQueue = -1;
    }
    else if (strcmp(cmd, "LIST") == 0)
    {
        char msg[MAX_MSG_SIZE];
        sprintf(msg, "L %d", myId);

        //send msg to server
        if (mq_send(serverQueue, msg, MAX_MSG_SIZE, LIST) == -1)
        {
            printf("can't send message\n");
            exit(1);
        }
    }
    else
    {
        //get the first part of input to check if it is CONNECT command
        char *buff = strtok(cmd, " ");
        if (strcmp(buff, "CONNECT") == 0)
        {
            //get other client id
            buff = strtok(NULL, " ");
            char msg[MAX_MSG_SIZE];
            sprintf(msg, "C %d %s", myId, buff);

            //send msg to server
            if (mq_send(serverQueue, msg, MAX_MSG_SIZE, CONNECT) == -1)
            {
                printf("can't send message\n");
                exit(1);
            }
        }
        else
        {
            printf("\nNo such command!\n");
        }
    }

    free(cmd);
}

void sendStop()
{
    //send STOP message to server
    char msg[MAX_MSG_SIZE];
    sprintf(msg, "S %d", myId);

    //send msg to server
    if (mq_send(serverQueue, msg, MAX_MSG_SIZE, STOP) == -1)
    {
        perror("Can't send message to server");
        exit(1);
    }
}

void proceedMsg(char *msg)
{
    char *type = strtok(msg, " ");

    //Stop message
    //message structure is: type
    if (strcmp(type, "S") == 0)
    {
        sendStop();
        exit(0);
    }
    //Disconnect message
    //message structure is: type
    else if (strcmp(type, "D") == 0)
    {
        //do nothing
        //it's just confirmation from server
    }
    //List message
    //message structure is: type o
    else if (strcmp(type, "L") == 0)
    {
        //do nothing
        //it's just confirmation from server
    }
    //Connect message
    //message structure is: type otherName
    else if (strcmp(type, "C") == 0)
    {
        printf("\nConnected with other client\n");

        char *otherName = strtok(NULL, " ");

        //open other's client queue
        otherQueue = mq_open(otherName, O_RDWR);
        
        if (otherQueue == -1)
        {
            perror("Can't open other queue");
            exit(1);
        }
    }
    //Init message
    //message structure is: type clientName
    else if (strcmp(type, "I") == 0)
    {
        if (!acitvated)
        { //if this client was't initialized activate it else ignore
            acitvated = TRUE;
            myId = atoi(strtok(NULL, " "));
        }
        else
        {
            printf("\nAnother init message received! Check server\n");
        }
    }
    else
    {
        printf("\nNo such message type\n");
    }
}


void registerNotify()
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;

    if (mq_notify(clientQueue, &sev) == -1)
    {
        perror("can't set up notification");
        exit(1);
    }
}



void sigintHandler()
{
    //send STOP message to server
    sendStop();
    exit(0); // just exit normally because atexit function is responsible to clean up
}

void sigusrHandler()
{
    char msg[MAX_MSG_SIZE_BUFF];

    if (mq_receive(clientQueue, msg, MAX_MSG_SIZE_BUFF, NULL) == -1)
    {
        perror("Can't receive message");
        removeQueue();
        exit(1);
    }

    registerNotify();

    proceedMsg(msg);


}

int main(int argc, char **argv)
{
    if (atexit(removeQueue) != 0)
    {
        perror("Can't set atexit function");
        return 1;
    }

    if (signal(SIGINT, sigintHandler) == SIG_ERR)
    {
        perror("Can't establish SIGINT handler");
        return 1;
    }

    if (signal(SIGUSR1, sigusrHandler) == SIG_ERR)
    {
        perror("Can't establish SIGUSR1 handler");
        return 1;
    }
    char name[15];

    sprintf(name, "%s%d", CLIENT_BASE, getpid());
    printf("%s\n", name);
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;

    clientQueue = mq_open(name, O_CREAT | O_RDWR | O_EXCL, QUEUE_PERMISSIONS, &attr);

    if (clientQueue == -1)
    {
        perror("Can't create client queue");
        exit(1);
    }

    init(name);
    registerNotify();

    char msg[MAX_MSG_SIZE_BUFF];

    while (TRUE)
    {
        if (mq_receive(clientQueue, msg, MAX_MSG_SIZE_BUFF, NULL) == -1)
        {
            perror("Can't receive message");
            removeQueue();
            exit(1);
        }

        proceedMsg(msg);

        run();
    }

    return 0;
}