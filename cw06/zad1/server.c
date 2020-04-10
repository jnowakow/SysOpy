#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "header.h"

int clientsIDs[MAX_CLIENTS];
int availability[MAX_CLIENTS];
key_t clientKeys[MAX_CLIENTS];
int nextID = 1; //ids begin from one to avoid initializing ids' array
int activeClients = 0;
int serverQueue = -1;
//int acitve = TRUE;

//inform all active clients and wait for STOP messages
void shutDown()
{
    //send stop message to all clients and wait for reponses
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientsIDs[i] > 0)
        {

            int clientQueue = msgget(clientKeys[i], IPC_CREAT);

            msgbuf msg;
            msg.mtype = STOP;

            if (msgsnd(clientQueue, &msg, MSG_SIZE, 0) != 0)
            {
                perror("Can't send stop message"); //if it's is't possible to send message to this client just remove the queue
                msgctl(serverQueue, IPC_RMID, NULL);
                exit(1);
            }

            //wait for returnig stop message from client
            if (msgrcv(clientQueue, &msg, MSG_SIZE, STOP, MSG_NOERROR) < 0)
            {
                perror("Shuting down, can't receive message");
                msgctl(serverQueue, IPC_RMID, NULL);
                exit(1);
            }
        }
    }

    if (msgctl(serverQueue, IPC_RMID, NULL) == -1)
    {
        perror("Can't remove server queue");
        exit(1);
    }
}

void sigintHandler()
{
    exit(0); // just exit normally because atexit function is responsible to remove queues
}

void init(key_t clientKey)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++){
        if (clientsIDs[i] == 0)
            break;
    }
    if (i == MAX_CLIENTS){
        printf("Maximal number of clients is logged\n");
        return;
    }

    int clientQueue = msgget(clientKey, IPC_CREAT);
    
    if (clientQueue == -1){
        printf("Can't open client queue\n");
        return; //there is no need to close server
    }

    msgbuf msg;
    msg.clientID = nextID;
    msg.mtype = INIT;
    if (msgsnd(clientQueue, &msg, MSG_SIZE, 0) != 0){
        printf("Can't resend init message\n"); 
        return;
    }

    clientsIDs[i] = nextID;
    clientKeys[i] = clientKey;
    availability[i] = TRUE;
    
    //inceremnt id 
    nextID++;
}

void list(){
    printf("---------------\n");
    printf("Client's id - availability\n");

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] > 0){
            printf("%d - %s\n", clientsIDs[i], (availability[i] == TRUE) ? "✓" : "✕" );
        }
    }
    printf("---------------\n");
}

void connect(int clientID, int withID){
    int from = 0, to = 0;
    //find clients in clients array
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] == clientID)
            from = i;

        if(clientsIDs[i] == withID)
            to = i;
        
        if(from != 0 && to != 0) 
            break;
    }

    //check if both clients are avaible
    if(availability[from] == FALSE || availability[to] == FALSE){
        printf("One of clinets is busy\n");
        return;
    }

    int fromKey = clientKeys[from];
    int toKey = clientKeys[to];

    //send msg to first client
    int queue = msgget(fromKey, IPC_CREAT);

    if(queue == -1){
        printf("can't open client queue\n");
        return;
    }

    msgbuf msg1;
    msg1.clientKey = toKey;
    msg1.mtype = CONNECT;

    if(msgsnd(queue, &msg1, MSG_SIZE, 0) != 0){
        printf("can't send message\n");
        return;
    } 

    //send msg to second client
    queue = msgget(toKey, IPC_CREAT);
    
    if(queue == -1){
        printf("can't open client queue\n");
        return;
    }

    msgbuf msg2;
    msg2.clientKey = fromKey;
    msg2.mtype = CONNECT;

    if(msgsnd(queue, &msg2, MSG_SIZE, 0) != 0){
        printf("can't send message\n");
        return;
    } 

    //set as not avaible
    availability[from] = FALSE;
    availability[to] = FALSE;
}

void disconnect(int clientID){
    int i = 0;
    for(; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] == clientID) 
            break;
    }

    availability[i] = TRUE;
}

void stop(int clientID){
    int i = 0;
    for(; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] == clientID) 
            break;
    }

    clientsIDs[i] = 0;
    clientKeys[i] = 0;
    availability[i] = FALSE;
}


void proceedMsg(msgbuf *msg)
{
    switch (msg->mtype){
    case STOP:
        printf("Removing %d from clients\n", msg->clientID);
        stop(msg->clientID);
        break;
    case DISCONNECT:
        printf("Disconnecting %d\n", msg->clientID);
        disconnect(msg->clientID);
        break;
    case LIST:
        list();
        break;
    case CONNECT:
        printf("Connecting %d with %s\n", msg->clientID, msg->mtext);
        connect(msg->clientID, atoi(msg->mtext));
        break;
    case INIT:
        printf("Got init message\n");
        init(msg->clientKey);
        break;
    default:
        printf("No such message type\n");
        break;
    }
}

int main(int argc, char **argv)
{
    if (atexit(shutDown) != 0){
        perror("Can't set atexit function");
        return 1;
    }

    if (signal(SIGINT, sigintHandler) == SIG_ERR){
        perror("Can't establish SIGINT handler");
        return 1;
    }

    char *path = getenv("HOME");
    key_t serverKey = ftok(path, KEY_SEED);

    if (serverKey == -1){
        perror("Can't create unique key");
        exit(1);
    }

    serverQueue = msgget(serverKey, IPC_CREAT | IPC_EXCL | 0666); //create serever queue, which should be unique

    if (serverQueue == -1){
        perror("Can't create server queue");
        exit(1);
    }

    msgbuf msg;

    while (TRUE){ //the only way to stop server is to send sigint to it
        //INIT is the less important signal so - before it means that the communicats
        //are taken with priority
        if (msgrcv(serverQueue, &msg, MSG_SIZE, -INIT, 0) == -1){
            perror("Can't receive message");
            shutDown(); //shutdown server
            exit(1);
        }

        proceedMsg(&msg);
    }

    return 0;
}