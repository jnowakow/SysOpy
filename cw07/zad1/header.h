#ifndef header
#define header

#define KEY getenv("HOME") //to generate key for shared memory with orders and semafore to synchronize access to it 
#define LOCK 213 //second argument to ftok

#define PERMISSIONS 0666
#define MAX_ORDERS_COUNT 10
#define TRUE 1
#define FALSE 0


typedef struct shared_orders{
    int orders[MAX_ORDERS_COUNT];
    int next_insert_idx;//index to place where insert next order
    int next_prepare_idx;//index of order to be prepared
    int next_send_idx;//index of order to be removed
    int to_prepare_count;
    int to_send_count;

} shared_orders;


typedef union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    #ifdef __linux__
    struct seminfo *__buf; //buffer for IPC_INFO (Linux-specific)
    #endif
    
} semun;





#endif