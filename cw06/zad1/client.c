#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "header.h"

int myId = -1;
int acitvated = FALSE;
int waitingForMsg = TRUE;
int serverQueue = -1;
int clientQueue = -1;
int otherQueue = -1;

void removeQueue()
{
    //remove queue
    if (msgctl(clientQueue, IPC_RMID, NULL) == -1)
    {
        perror("Can't remove queue");
        exit(1);
    }
}

void init(key_t clientKey, char *path)
{
    //open serever queue and pass key
    key_t serverKey = ftok(path, KEY_SEED);
    if (serverKey == -1)
    {
        perror("Can't get server key");
        exit(1);
    }

    serverQueue = msgget(serverKey, IPC_CREAT);
    if (serverQueue == -1)
    {
        perror("Can't open server queue");
        exit(1);
    }

    msgbuf msg;
    msg.mtype = INIT;
    msg.clientKey = clientKey;
    msg.clientPid = getpid();

    if (msgsnd(serverQueue, &msg, MSG_SIZE, 0) != 0)
    {
        printf("can't send message\n");
        exit(1);
    }
}

//read commands from stdin
void run()
{
    char *cmd = (char *)calloc(MAX_LEN, sizeof(char));

    fgets(cmd, MAX_LEN, stdin);
    cmd = strtok(cmd, "\n");
    if (strcmp(cmd, "STOP") == 0)
    {
        msgbuf msg;
        msg.clientID = myId;
        msg.mtype = STOP;

        //send msg to server
        if (msgsnd(serverQueue, &msg, MSG_SIZE, 0) != 0)
        {
            perror("Can't stop");
            exit(1); //TODO handle error
        }
        //end work

        exit(0);
    }
    else if (strcmp(cmd, "DISCONNECT") == 0)
    {
        msgbuf msg;
        msg.clientID = myId;
        msg.mtype = DISCONNECT;

        //send msg to server
        if (msgsnd(serverQueue, &msg, MSG_SIZE, 0) != 0)
        {
            printf("can't send message\n");
            exit(1);
        }

        waitingForMsg = FALSE;
    }
    else if (strcmp(cmd, "LIST") == 0)
    {
        msgbuf msg;
        msg.clientID = myId;
        msg.mtype = LIST;

        //send msg to server
        if (msgsnd(serverQueue, &msg, MSG_SIZE, 0) != 0)
        {
            printf("can't send message\n");
            exit(1);
        }

        waitingForMsg = FALSE;
    }
    else
    {
        //get the first part of input to check if it is CONNECT command
        char *buff = strtok(cmd, " ");
        if (strcmp(buff, "CONNECT") == 0)
        {
            //get other client id
            buff = strtok(NULL, " ");
            msgbuf msg;
            msg.clientID = myId;
            msg.mtype = CONNECT;
            strcpy(msg.mtext, buff);

            //send msg to server
            if (msgsnd(serverQueue, &msg, MSG_SIZE, 0) != 0)
            {
                printf("can't send message\n");
                exit(1);
            }

            waitingForMsg = TRUE;
        }
        else
        {
            printf("\nNo such command!\n");
            waitingForMsg = FALSE;
        }
    }

    free(cmd);
}
void sendStop()
{
    //send STOP message to server
    msgbuf msg2;
    msg2.mtype = STOP;
    msg2.clientID = myId;

    if (msgsnd(serverQueue, &msg2, MSG_SIZE, 0) < 0)
    {
        perror("Can't send message to server");
        exit(1);
    }
}

void proceedMsg(msgbuf *msg)
{
    switch (msg->mtype)
    {
    case INIT:
        if (!acitvated)
        { //if this client was't initialized activate it else ignore
            acitvated = TRUE;
            myId = msg->clientID;
            waitingForMsg = FALSE;
        }
        else
        {
            printf("\nAnother init message received! Check server\n");
        }

        break;
    case STOP:
        sendStop();
        exit(0);

        break;
    case CONNECT:
        printf("\nConnected with other client\n");
        //open other's client queue
        otherQueue = msgget(msg->clientKey, IPC_CREAT);

        if (otherQueue == -1)
        {
            perror("Can't open other queue");
            exit(1);
        }

        waitingForMsg = FALSE;

        break;
        

    default:
        break;
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
    msgbuf msg;
    
    if (msgrcv(clientQueue, &msg, MSG_SIZE, -INIT, 0) == -1)
    {
        perror("Can't receive message");
        removeQueue();
        exit(1);
    }
    
    proceedMsg(&msg);

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
        perror("Can't establish SIGINT handler");
        return 1;
    }

    char *path = getenv("HOME");
    //instruction says that there should be some number from header.h
    //but it's hard to guarantee uniquess, because how should client
    //know which number was used?
    key_t clientKey = ftok(path, getpid());
    if (clientKey == -1)
    {
        perror("Can't create unique key");
        exit(1);
    }

    clientQueue = msgget(clientKey, IPC_CREAT | IPC_EXCL | 0666);

    if (clientQueue == -1)
    {
        perror("Can't create client queue");
        exit(1);
    }

    init(clientKey, path);

    msgbuf msg;

    while (TRUE)
    {
        //consider message priority so -INIT as type
        if (waitingForMsg)
        {
            if (msgrcv(clientQueue, &msg, MSG_SIZE, -INIT, 0) == -1)
            {
                perror("Can't receive message");
                removeQueue();
                exit(1);
            }

            proceedMsg(&msg);
        }
        else
        {
            run();
        }
    }

    return 0;
}