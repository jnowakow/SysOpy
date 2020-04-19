#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "header.h"

int clientsIDs[MAX_CLIENTS];
int availability[MAX_CLIENTS];
mqd_t clientsDescs[MAX_CLIENTS];
char clientsNames[MAX_CLIENTS][15];//to be able to pass name to other client
int nextID = 1; //ids begin from one to avoid initializing ids' array
mqd_t serverQueue = -1;

//inform all active clients and wait for STOP messages
void removeQueue()
{
    //send stop message to all clients and wait for reponses
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientsIDs[i] > 0)
        {

            char msg[MAX_MSG_SIZE] = "S ";

            if (mq_send(clientsDescs[i], msg, sizeof(msg), STOP) == -1)
            {
                perror("Can't send stop message"); //if it's is't possible to send message to this client just remove the queue
                exit(1);
            }

            //wait for returnig stop message from client
            if (mq_receive(serverQueue, msg, MAX_MSG_SIZE_BUFF, NULL) == -1)
            {
                perror("Shuting down, can't receive message");
                exit(1);
            }

            mq_close(clientsDescs[i]);
        }
    }

    if (mq_close(serverQueue) == -1)
    {
        perror("Can't close server queue");
        exit(1);
    }

    if(mq_unlink(SERVER_NAME) == -1){
        perror("Can't remove server queue");
        exit(1);
    }

}

void sigintHandler()
{
    exit(0); // just exit normally because atexit function is responsible to remove queues
}

void init(char *clientName)
{
    //printf("%s\n", clientName);
    int i;
    for (i = 0; i < MAX_CLIENTS; i++){
        if (clientsIDs[i] == 0)
            break;
    }
    if (i == MAX_CLIENTS){
        printf("Maximal number of clients is logged\n");
        return;
    }

    strcpy(clientsNames[i], clientName);
    clientsIDs[i] = nextID;
    availability[i] = TRUE;
    clientsDescs[i] = mq_open(clientName, O_WRONLY);
    
    if(clientsDescs[i] == -1){
        perror("Can't open client's queue");
        clientsIDs[i] = 0;
        exit(1);
    }

    //send message back to client with his id
    char msg[MAX_MSG_SIZE];
    sprintf(msg, "I %d", nextID);

    if(mq_send(clientsDescs[i], msg, MAX_MSG_SIZE, INIT) == -1){
        perror("Can't send init message to client");
        exit(1);
    }
    
    //inceremnt id 
    nextID++;
}

void list(int clientId){
    printf("---------------\n");
    printf("Client's id - availability\n");

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] > 0){
            printf("%d          -  %s\n", clientsIDs[i], (availability[i] == TRUE) ? "✓" : "✕" );
        }
    }
    printf("---------------\n");

    int i = 0;
    for(; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] == clientId) 
            break;
    }

    char msg[] = "L ";
    
    if(mq_send(clientsDescs[i], msg, sizeof(msg), LIST) == -1){
        perror("Can't send init message to client");
        exit(1);
    }

    
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

    //connect message structure is: type otherClientName

    //send message to first client
    char msg1[MAX_MSG_SIZE];
    sprintf(msg1, "C %s", clientsNames[to]);
    printf("%s\n", msg1);

    if(mq_send(clientsDescs[from], msg1, MAX_MSG_SIZE, CONNECT) == -1){
        perror("Can't send message");
        exit(1);
    }

    char msg2[MAX_MSG_SIZE];
    sprintf(msg2, "C %s", clientsNames[from]);
    printf("%s\n", msg2);

    if(mq_send(clientsDescs[to], msg2, MAX_MSG_SIZE, CONNECT) == -1){
        perror("Can't send message");
        exit(1);
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
    
    char msg[] = "D ";
    
    if(mq_send(clientsDescs[i], msg, sizeof(msg), DISCONNECT) == -1){
        perror("Can't send init message to client");
        exit(1);
    }

}

void stop(int clientID){
    int i = 0;
    for(; i < MAX_CLIENTS; i++){
        if(clientsIDs[i] == clientID) 
            break;
    }
    
    mq_close(clientsDescs[i]);

    clientsIDs[i] = 0;
    clientsDescs[i] = -1;
    //clientsNames[i] = NULL;
    availability[i] = FALSE;
}

//first byte of message is it's type but messages structures differ depending on type
void proceedMsg(char *msg)
{
    //printf("%s\n", msg);
    char *type = strtok(msg, " ");
    //printf("%s\n", type);
    //Stop message
    //message structure is: type clientId
    if(strcmp(type, "S") == 0){
        int clientId = atoi(strtok(NULL, " "));
        printf("\nRemoving %d from clients\n", clientId);
        stop(clientId);
    }
    //Disconnect message
    //message structure is: type clientId
    else if(strcmp(type, "D") == 0){
        int clientId = atoi(strtok(NULL, " "));
        printf("\nDisconnecting %d\n", clientId);
        disconnect(clientId);
    }
    //List message
    //message structure is: type clientId
    else if (strcmp(type, "L") == 0)
    {
        int clientId = atoi(strtok(NULL, " "));
        list(clientId);
    }
    //Connect message
    //message structure is: type clientId otherClientId
    else if (strcmp(type, "C") == 0)
    {   
        int clientId = atoi(strtok(NULL, " "));
        int otherId = atoi(strtok(NULL, " "));
        connect(clientId, otherId);
    }
    //Init message
    //message structure is: type clientName
    else if (strcmp(type, "I") == 0)
    {   
        printf("Logging new client\n");
        char *clientName = strtok(NULL, " ");
        init(clientName);
    }
    else{
        printf("\nNo such message type\n");
    }
}

int main(int argc, char **argv)
{
    if (atexit(removeQueue) != 0){
        perror("Can't set atexit function");
        return 1;
    }

    if (signal(SIGINT, sigintHandler) == SIG_ERR){
        perror("Can't establish SIGINT handler");
        return 1;
    }
    
    for(int i = 0; i < MAX_CLIENTS; i++){
        clientsDescs[i] = -1;
    }

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    

    serverQueue = mq_open(SERVER_NAME, O_RDONLY | O_CREAT | O_EXCL , QUEUE_PERMISSIONS, &attr);
    
    if (serverQueue == -1){
        perror("Can't create server queue");
        exit(1);
    }

    char msg[MAX_MSG_SIZE_BUFF];

    while (TRUE){ //the only way to stop server is to send sigint to it
        if (mq_receive(serverQueue, msg, MAX_MSG_SIZE_BUFF, NULL ) == -1){
            perror("Can't receive message");
            exit(1);
        }

        proceedMsg(msg);
    }

    return 0;
}