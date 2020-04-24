#ifndef header
#define header

#define MAX_ORDERS_COUNT 128
#define ORDERS_KEY "/bin" //to generate key for shared memory with orders and semafore to synchronize access to it 
#define ADD_KEY "/root" // to generate key for semaphore indicating if next order can be added
#define PREPARE_KEY "/lib" //to generate key for semaphore indicating if next order can be prepared
#define SEND_KEY "/etc" //to generate key for semaphore indicating if next order can be removed
#define PROJECT_ID 113
#define TRUE 1
#define FALSE 0


typedef struct shared_orders{
    int orders[MAX_ORDERS_COUNT];
    int next_insert_idx;//index to place where insert next order
    int next_prepare_idx;//index of order to be prepared
    int next_send_idx;//index of order to be removed
    int size;
    int to_prepare;
    int to_send;

} shared_orders;

typedef struct shared_orders_to_send{
    int orders[MAX_ORDERS_COUNT];
    int next_prepare_idx;
    int next_insert_idx;
    int size;

} shared_orders_to_send;

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