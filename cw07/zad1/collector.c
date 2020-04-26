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

void handler()
{
    exit(0);
}

void logger(pid_t pid, int order_size, int to_prepare, int to_send)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    printf("(%d %lds %ldms) Dostałem liczbę %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n",
           pid, tm.tv_sec, tm.tv_nsec % 1000, order_size, to_prepare, to_send);
}

void detach_shm()
{
    if (shmdt((void *)orders) == -1)
    {
        perror("shmdt");
        exit(1);
    }
}

//all semaphores and shared memory segments are created by driver program
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "This program takes seed of random number generator as an argument\n");
        return 1;
    }

    //printf("HELLO!\n");
    int seed = atoi(argv[1]);
    int locker_sem;
    int shm_id;
    key_t key;
    pid_t pid = getpid();

    //at exit detach memory
    if (atexit(detach_shm) == -1)
    {
        perror("atexit");
        return 1;
    }

    if (signal(SIGUSR1, handler) == SIG_ERR)
    {
        perror("signal");
        return 1;
    }

    
    key = ftok(KEY, LOCK);
    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    locker_sem = semget(key, 1, IPC_CREAT);
    if (locker_sem == -1)
    {
        perror("locker semaphore");
        return 1;
    }

    shm_id = shmget(key, sizeof(shared_orders), IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget");
        return 1;
    }

    orders = (shared_orders *)shmat(shm_id, NULL, 0);
    if (orders == (shared_orders *)-1)
    {
        perror("shmat");
        return 1;
    }

    srand(seed ^ (pid << 16));

    struct sembuf asem;

    asem.sem_num = 0;
    asem.sem_op = 0;
    asem.sem_flg = 0;


    while (TRUE)
    {
        int order_size = rand() % 100 + 1; //get random size of order
        
        //decrement locker to block access for other processes
        asem.sem_op = -1;

        if (semop(locker_sem, &asem, 1) == -1)
        {
            perror("semop locker");
            return 1;
        }

        //if array is full increment can_add_sem to prevent from inserting new order
        if (orders->to_prepare_count + orders->to_send_count == MAX_ORDERS_COUNT)
        {
            asem.sem_op = 1;

            if (semop(locker_sem, &asem, 1) == -1)
            {
                perror("semop locker");
                return 1;
            }

            continue;
        }

        orders->orders[orders->next_insert_idx] = order_size;
        (orders->next_insert_idx)++;

        orders->next_insert_idx = orders->next_insert_idx % MAX_ORDERS_COUNT;

        (orders->to_prepare_count)++;

        
        logger(pid, order_size, orders->to_prepare_count, orders->to_send_count);

        //release the share memory
        asem.sem_op = 1;

        if (semop(locker_sem, &asem, 1) == -1)
        {
            perror("semop locker");
            return 1;
        }

        sleep((rand() % 3) + 1);
    }

    return 0;
}