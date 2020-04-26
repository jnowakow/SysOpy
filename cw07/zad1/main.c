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

int collectors;
pid_t *collectors_pids;

int packers;
pid_t *packers_pids;

int senders;
pid_t *senders_pids;

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
    free(collectors_pids);
    free(packers_pids);
    free(senders_pids);

    if(shmdt(orders_ptr) == -1)
        error("shmdt");

    if(shmctl(shm_id, IPC_RMID, NULL) == -1)
        error("shmctl");
    
    if(semctl(locker_sem, 0, IPC_RMID) == -1)
        error("semctl");

}

void sigint_handler()
{
    for (int i = 0; i < collectors; i++)
    {
        if (kill(collectors_pids[i], SIGUSR1) == -1)
            error("kill");
    }

    for (int i = 0; i < packers; i++)
    {
        if (kill(packers_pids[i], SIGUSR1) == -1)
            error("kill");
    }

    for (int i = 0; i < senders; i++)
    {
        if (kill(senders_pids[i], SIGUSR1) == -1)
            error("kill");
    }

    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        error("signal");

    if (atexit(clean_up) == -1)
        error("atexit");

    collectors = atoi(argv[1]);
    collectors_pids = (pid_t *)calloc(collectors, sizeof(pid_t));
    
    packers = atoi(argv[2]);
    packers_pids = (pid_t *)calloc(packers, sizeof(pid_t));
    
    senders = atoi(argv[3]);
    senders_pids = (pid_t *)calloc(senders, sizeof(pid_t));
    
    
    key_t key;
    semun sem_attr;

    key = ftok(KEY, LOCK);
    if (key == -1)
        error("ftok");

    /*** SHARED MEMORY ***/
    shm_id = shmget(key, sizeof(shared_orders), PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (shm_id == -1)
        error("shmget");

    orders_ptr = (shared_orders *)shmat(shm_id, NULL, 0);
    if (orders_ptr == (shared_orders *)-1)
        error("shmat");

    //initialize shared memory
    orders_ptr->next_insert_idx = orders_ptr->next_prepare_idx = orders_ptr->next_send_idx = 0;
    orders_ptr->to_prepare_count = orders_ptr->to_send_count = 0;

   
    /*** SEMAPHORE TO BLOCK SHARED MEMORY IF SOME PROCESS IS MODIFYING IT ***/
    locker_sem = semget(key, 1, PERMISSIONS | IPC_CREAT | IPC_EXCL);
    if (locker_sem == -1)
        error("locker semaphore");

    //set memory ready to use
    sem_attr.val = 1;
    if (semctl(locker_sem, 0, SETVAL, sem_attr) == -1)
        error("semctl SETVAL");

    
    srand(time(NULL));
    char seed[6];

    for (int i = 0; i < collectors; i++){
        collectors_pids[i] = fork();
        if(collectors_pids[i] == 0){
            sprintf(seed, "%d", rand() % 100000 + 1);
            execl("./collector", "./collector", seed, NULL);
        }
    }

    for (int i = 0; i < packers; i++){
        packers_pids[i] = fork();
        if(packers_pids[i] == 0){
            execl("./packer", "./packer", NULL);
        }
    }

    for (int i = 0; i < senders; i++){
        senders_pids[i] = fork();
        if(senders_pids[i] == 0){
            execl("./sender", "./sender", NULL);
        }
    }

    for (int i = 0; i < collectors; i++){
        waitpid(collectors_pids[i], NULL, 0);
    }

    for (int i = 0; i < collectors; i++){
        waitpid(packers_pids[i], NULL, 0);
    }

    for (int i = 0; i < senders; i++){
        waitpid(senders_pids[i], NULL, 0);
    }

    return 0;
}