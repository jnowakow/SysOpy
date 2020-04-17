#ifndef header
#define header

#define MAX_CLIENTS 8 //maximal number of clinets
#define MAX_LEN 256 //maximal length of message
#define KEY_SEED 113 //seed for ftok to generate key for server queue
#define TRUE 1
#define FALSE 0

//for messages bettwen server and clinet
typedef struct msgbuf{
    long mtype;
    char mtext[MAX_LEN];
    int clientID;
    key_t clientKey;
} msgbuf;

//types of messages between server and client
//last two types are for comunnication between clients
typedef enum type{
    STOP = 1, DISCONNECT = 2, LIST = 3, CONNECT = 4, INIT = 5
} type;

const size_t MSG_SIZE = sizeof(msgbuf) - sizeof(long);

#endif