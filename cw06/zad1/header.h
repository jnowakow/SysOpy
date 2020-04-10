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
typedef enum serv_cli_type{
    STOP = 0, DISCONNECT = 1, LIST = 2, CONNECT = 3, INIT = 4
} serv_cli_type;

typedef struct clientsMsg
{
    long mtype;
    char mtext[MAX_LEN];
} clientsMsg;

typedef enum cli_cli_type{
    END = 0, NORMAL = 1
} cli_cli_type;





const size_t MSG_SIZE = sizeof(msgbuf) - sizeof(long);

#endif