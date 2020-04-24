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
    if (argc != 1)
    {
        fprintf(stderr, "This program takes no arguments\n");
        return 1;
    }

    //printf("HELLO!\n");

    int can_add_sem;
    int can_prepare_sem;
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

    key = ftok(KEY, ADD);
    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    can_add_sem = semget(key, 1, IPC_CREAT);
    if (can_add_sem == -1)
    {
        perror("semget");
        return 1;
    }

    key = ftok(KEY, PREPARE);
    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    can_prepare_sem = semget(key, 1, IPC_CREAT);
    if (can_prepare_sem == -1)
    {
        perror("semget ");
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

    time_t seed = (time_t) getpid() * 1000;
    srand(time(&seed));

    struct sembuf asem;

    asem.sem_num = 0;
    asem.sem_op = 0;
    asem.sem_flg = 0;

    semun sem_attr;
    

    while (TRUE)
    {
        int order_size = rand() % 100 + 1; //get random size of order
        
        asem.sem_op = 0;
        //wait till next order can be added
        if (semop(can_add_sem, &asem, 1) == -1)
        {
            perror("semop add_sem");
            return 1;
        }

        //decrement locker to block access for other processes
        asem.sem_op = -1;

        if (semop(locker_sem, &asem, 1) == -1)
        {
            perror("semop locker");
            return 1;
        }
        
        orders->orders[orders->next_insert_idx] = order_size;
        (orders->next_insert_idx)++;

        orders->next_insert_idx = orders->next_insert_idx % MAX_ORDERS_COUNT;

        (orders->size)++;
        (orders->to_prepare)++;

        //if array is full increment can_add_sem to prevent from inserting new order
        if (orders->size == MAX_ORDERS_COUNT)
        {
            asem.sem_op = 1;

            if (semop(can_add_sem, &asem, 1) == -1)
            {
                perror("semop add_sem");
                return 1;
            }
        }

        //mark that there is order to prepare
        sem_attr.val = 0;
        if (semctl(can_prepare_sem, 0, SETVAL, sem_attr) == -1){
            perror("semctl SETVAL");
            exit(1);
        }

        logger(pid, order_size, orders->to_prepare, orders->to_send);

        //release the share memory
        asem.sem_op = 1;

        if (semop(locker_sem, &asem, 1) == -1)
        {
            perror("semop locker");
            return 1;
        }

        sleep(1);
    }

    return 0;
}