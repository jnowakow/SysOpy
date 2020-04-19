#ifndef header
#define header

#define MAX_CLIENTS 8 //maximal number of clinets
#define SERVER_NAME "/server"
#define CLIENT_BASE "/client"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MAX_MSG_SIZE_BUFF MAX_MSG_SIZE + 10 //when receving messgaes the buffer must be greater than max message size
#define TRUE 1
#define FALSE 0


typedef enum priority{
    INIT = 0, CONNECT = 1, LIST = 2, DISCONNECT = 3, STOP = 4 
} priority;

#endif