#ifndef header
#define header

#define SHM_NAME "/shm" //name of shared memory with orders 
#define LOCKER_NAME "/locker" // name of semaphore used to synchronize shared memory access

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


#endif