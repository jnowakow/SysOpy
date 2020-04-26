#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <signal.h>
#include <time.h>

#include "header.h"

shared_orders *orders;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void handler()
{
    exit(0);
}

void logger(pid_t pid, int order_size, int to_prepare, int to_send)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    printf("(%d %lds %ldms) Przygotowałem zamówienie o wielkości %d(2 * %d). Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n",
           pid, tm.tv_sec, tm.tv_nsec % 1000, 2*order_size, order_size,to_prepare, to_send);
}

void detach_shm()
{
    if (shmdt((void *)orders) == -1)
        error("shmdt");
}

//all semaphores and shared memory segments are created by driver program
int main(int argc, char **argv)
{
    if (argc != 1)
    {
        fprintf(stderr, "This program takes no arguments\n");
        return 1;
    }

    int locker_sem;
    int shm_id;
    key_t key;
    pid_t pid = getpid();

    //at exit detach memory
    if (atexit(detach_shm) == -1)
        error("atexit");

    if (signal(SIGUSR1, handler) == SIG_ERR)
        error("signal");    

    //*** semaphore locking shared memory if process is using it ***//
    key = ftok(KEY, LOCK);
    if (key == -1)
        error("ftok");
    
    locker_sem = semget(key, 1, IPC_CREAT);
    if (locker_sem == -1)
        error("locker semaphore");
        
    //***          shared memory                              ***//
    shm_id = shmget(key, sizeof(shared_orders), IPC_CREAT);
    if (shm_id == -1)
        error("shmget");
        

    orders = (shared_orders *)shmat(shm_id, NULL, 0);
    if (orders == (shared_orders *)-1)
        error("shmat");
        


    struct sembuf asem;

    asem.sem_num = 0;
    asem.sem_op = 0;
    asem.sem_flg = 0;


    int order_size;    

    while (TRUE)
    {
        //decrement locker to block access for other processes
        asem.sem_op = -1;

        if (semop(locker_sem, &asem, 1) == -1)
            error("semop locker");
        
        // if there are no orders to prepare release memory
        if (orders->to_prepare_count == 0){
            asem.sem_op = 1;

            if (semop(locker_sem, &asem, 1) == -1)
                error("semop locker");
            
            continue;
        }

        
        order_size =  orders->orders[orders->next_prepare_idx];
        (orders->next_prepare_idx)++;

        orders->next_prepare_idx = orders->next_prepare_idx % MAX_ORDERS_COUNT;


        (orders->to_prepare_count)--;
        (orders->to_send_count)++;

        logger(pid, order_size, orders->to_prepare_count, orders->to_send_count);

        //release the share memory
        asem.sem_op = 1;

        if (semop(locker_sem, &asem, 1) == -1)
            error("semop locker");
            

        sleep((rand() % 3) + 1);
    }

    return 0;
}