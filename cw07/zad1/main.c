#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>
#include <time.h>

#include "header.h"

int worker1;
pid_t *worker1_pids;

int can_send_sem;
int can_add_sem;
int can_prepare_sem;
int locker_sem;

int shm_id;
shared_orders *orders_ptr;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

//remove shared memory and semaphores
void clean_up()
{
    free(worker1_pids);

    if(shmdt(orders_ptr) == -1)
        error("shmdt");

    if(shmctl(shm_id, IPC_RMID, NULL) == -1)
        error("shmctl");
    
    if(semctl(can_add_sem, 0, IPC_RMID) == -1)
        error("semctl");

    if(semctl(can_prepare_sem, 0, IPC_RMID) == -1)
        error("semctl");
    
    if(semctl(can_send_sem, 0, IPC_RMID) == -1)
        error("semctl");
    
    if(semctl(locker_sem, 0, IPC_RMID) == -1)
        error("semctl");

}

void sigint_handler()
{
    for (int i = 0; i < worker1; i++)
    {
        if (kill(worker1_pids[i], SIGUSR1) == -1)
            error("kill");
    }

    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        error("signal");

    if (atexit(clean_up) == -1)
        error("atexit");

    worker1 = atoi(argv[1]);
    worker1_pids = (pid_t *)calloc(worker1, sizeof(pid_t));
    key_t key;

    semun sem_attr;

    /*** SEMAPHORE INDIDATING IF THERE ARE ORDERS TO SEND ***/
    key = ftok(KEY, SEND);
    if (key == -1)
        error("ftok");

    can_send_sem = semget(key, 1, PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (can_send_sem == -1)
        error("semget");

    //block sending oreders, becasue there are none
    sem_attr.val = 1;
    if (semctl(can_send_sem, 0, SETVAL, sem_attr) == -1)
        error("semctl SETVAL");

    /*** SEMAPHORE INDIDATING IF ORDERS CAN BE ADDED ***/

    key = ftok(KEY, ADD);
    if (key == -1)
        error("ftok");

    can_add_sem = semget(key, 1, PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (can_add_sem == -1)
        error("semget");
    
    //set array available for new orders
    sem_attr.val = 0;
    if (semctl(can_add_sem, 0, SETVAL, sem_attr) == -1)
        error("semctl SETVAL");

    /*** SEMAPHORE INDIDATING IF THERE ARE ORDERS TO PREPARE ***/

    key = ftok(KEY, PREPARE);
    if (key == -1)
        error("ftok");

    can_prepare_sem = semget(key, 1, PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (can_prepare_sem == -1)
        error("semget ");

    //block preparing orderes because there are none
    sem_attr.val = 1;
    if (semctl(can_prepare_sem, 0, SETVAL, sem_attr) == -1)
        error("semctl SETVAL");

    /*** SEMAPHORE TO BLOCK SHARED MEMORY IF SOME PROCESS IS MODIFYING IT ***/

    key = ftok(KEY, LOCK);
    if (key == -1)
        error("ftok");

    locker_sem = semget(key, 1, PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (locker_sem == -1)
        error("locker semaphore");

    //set memory ready to use
    sem_attr.val = 1;
    if (semctl(locker_sem, 0, SETVAL, sem_attr) == -1)
        error("semctl SETVAL");

    /*** SHARED MEMORY ***/

    shm_id = shmget(key, sizeof(shared_orders), PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (shm_id == -1)
        perror("shmget");

    orders_ptr = (shared_orders *)shmat(shm_id, NULL, 0);
    if (orders_ptr == (shared_orders *)-1)
        error("shmat");

    //initialize shared memory
    orders_ptr->next_insert_idx = orders_ptr->next_prepare_idx = orders_ptr->next_send_idx = 0;
    orders_ptr->to_prepare = orders_ptr->to_send = 0;
    orders_ptr->size = 0;

    
    for (int i = 0; i < worker1; i++){
        worker1_pids[i] = fork();
        if(worker1_pids[i] == 0){
            execl("./worker1", "./worker1", NULL);
        }
    }

    for (int i = 0; i < worker1; i++){
        waitpid(worker1_pids[i], NULL, 0);
    }


    return 0;
}