#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include <semaphore.h>

#include <signal.h>
#include <time.h>

#include "header.h"

shared_orders *orders_ptr;

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

void unmap_shm()
{
    if(munmap(orders_ptr, sizeof(shared_orders)) == -1)
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

    sem_t *locker_sem;
    int shm_desc;
    
    pid_t pid = getpid();

    //at exit detach memory
    if (atexit(unmap_shm) == -1)
        error("atexit");
        
    if (signal(SIGUSR1, handler) == SIG_ERR)
        error("signal");
        

    locker_sem = sem_open(LOCKER_NAME, O_RDWR);
    if (locker_sem == SEM_FAILED)
        error("locker semaphore");
        
    shm_desc = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_desc == -1)
        error("shmget");
        

    orders_ptr = mmap(NULL, sizeof(shared_orders), PROT_READ | PROT_WRITE, MAP_SHARED, shm_desc, 0);
    if (orders_ptr == MAP_FAILED)
        error("shmat");


    int order_size;    

    while (TRUE)
    {
        //decrement locker to block access for other processes

        if (sem_wait(locker_sem) == -1)
            error("sem_wait locker");
        
        // if there are no orders to prepare release memory
        if (orders_ptr->to_prepare_count == 0){
            
            if (sem_post(locker_sem) == -1)
                error("semop locker");
            
            continue;
        }

        
        order_size =  orders_ptr->orders[orders_ptr->next_prepare_idx];
        (orders_ptr->next_prepare_idx)++;

        orders_ptr->next_prepare_idx = orders_ptr->next_prepare_idx % MAX_ORDERS_COUNT;


        (orders_ptr->to_prepare_count)--;
        (orders_ptr->to_send_count)++;

        logger(pid, order_size, orders_ptr->to_prepare_count, orders_ptr->to_send_count);

        //release the share memory
        
        if (sem_post(locker_sem) == -1)
            error("semop locker");
            

        sleep((rand() % 3) + 1);
    }

    return 0;
}