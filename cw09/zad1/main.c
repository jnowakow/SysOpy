#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int max_chairs_num; //maxium number of waiting chairs
int current_client_id = -1;

int taken_chairs_num = 0; //number of taken chairs
pthread_mutex_t chairs_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t chairs_cond = PTHREAD_COND_INITIALIZER;

int barber_is_sleeping = FALSE;
pthread_mutex_t sleeping_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sleeping_cond = PTHREAD_COND_INITIALIZER;

int barber_is_free = TRUE;
pthread_mutex_t free_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t free_cond = PTHREAD_COND_INITIALIZER;

int func_result;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void *barber(void *args)
{
    int chairs_num_copy;
    
    while (TRUE)
    {
        //check if there are clients
        func_result = pthread_mutex_lock(&chairs_mutex);
        if (func_result != 0)
            error("mutex lock");

        //there is no clients
        if (taken_chairs_num == 0)
        {
            chairs_num_copy = taken_chairs_num;
            //release chairs mutex
            func_result = pthread_mutex_unlock(&chairs_mutex);
            if (func_result != 0)
                error("mutex unlock");

            //go to sleep and wait till there is new client
            func_result = pthread_mutex_lock(&sleeping_mutex);
            if (func_result != 0)
                error("mutex lock");

            printf("Golibroda: idę spać\n");
            barber_is_sleeping = TRUE;

            while (barber_is_sleeping)
            {
                pthread_cond_wait(&sleeping_cond, &sleeping_mutex);
            }

            func_result = pthread_mutex_unlock(&sleeping_mutex);
            if (func_result != 0)
                error("mutex unlock");
        }
        //let next client in 
        else
        {
            taken_chairs_num--;
            chairs_num_copy = taken_chairs_num;
            //release chairs mutex
            func_result = pthread_mutex_unlock(&chairs_mutex);
            if (func_result != 0)
                error("mutex unlock");
        }
        
        //shave client
        printf("Golibroda: czeka %d klientów, gole klienta %d\n", chairs_num_copy, current_client_id);
        sleep(rand() % 5 + 1);

        //inform next client that place is avaible 
        func_result = pthread_mutex_lock(&free_mutex);
        if (func_result != 0)
            error("mutex lock");

        barber_is_free = TRUE;
        pthread_cond_signal(&free_cond);

        func_result = pthread_mutex_unlock(&free_mutex);
        if (func_result != 0)
            error("mutex unlock");
    }

    return NULL;
}

void *client(void *arg)
{
    int my_id = *((int *)arg);
    int woken_barber = FALSE;//flag indicating if this thread has woken barber
    
    //check if barber is sleeping
    func_result = pthread_mutex_lock(&sleeping_mutex);
    if (func_result != 0)
        error("mutex lock");

    if (barber_is_sleeping)
    {   
        printf("%d: budzę golibrodę\n", my_id);
        //set flag appropriately
        woken_barber = TRUE;

        barber_is_sleeping = FALSE;
        current_client_id = my_id;
        pthread_cond_signal(&sleeping_cond);
    }

    func_result = pthread_mutex_unlock(&sleeping_mutex);
    if (func_result != 0)
        error("mutex unlock");

    //if barber wasn't sleeping 
    if (!woken_barber)
    {
        //continue checking if there is new free place
        while (TRUE)
        {
            func_result = pthread_mutex_lock(&chairs_mutex);
            if (func_result != 0)
                error("mutex lock");

            //if there are no free places
            if (taken_chairs_num == max_chairs_num)
            {
                //release chairs mutex
                func_result = pthread_mutex_unlock(&chairs_mutex);
                if (func_result != 0)
                    error("mutex unlock");

                printf("%d: Zajęte\n", my_id);
                //wait random number of seconds
                sleep(rand() % 3 + 1);
            }
            //if there are free places
            else
            {
                taken_chairs_num++;
                printf("%d: Poczekalnia, wolne miejsca - %d\n", my_id, (max_chairs_num - taken_chairs_num));
                //release places
                func_result = pthread_mutex_unlock(&chairs_mutex);
                if (func_result != 0)
                    error("mutex unlock");
                break;
            }
        }

        //if thread is waiting in queue wait for signal from barber
        func_result = pthread_mutex_lock(&free_mutex);
        if (func_result != 0)
            error("mutex lock");

        while (!barber_is_free)
        {
            pthread_cond_wait(&free_cond, &free_mutex);
        }
        barber_is_free = FALSE;
        
        //go to shave bread
        current_client_id = my_id;
        
        func_result = pthread_mutex_unlock(&free_mutex);
        if (func_result != 0)
            error("mutex unlock");
    }

    return NULL;
}
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of argumnets! Usage is ./main K N\n");
        return 1;
    }

    max_chairs_num = atoi(argv[1]);
    int N = atoi(argv[2]);

    //create barber
    pthread_t barber_id;
    func_result = pthread_create(&barber_id, NULL, barber, NULL);
    if (func_result != 0)
        error("pthread create");

    pthread_t clients_id[N];

    //create clients
    for (int i = 0; i < N; i++)
    {
        func_result = pthread_create(&clients_id[i], NULL, client, (void *) &i);
        if (func_result != 0)
            error("pthread create");
        
        sleep(rand() % 2 + 1);
    }

    //wait for clients to terminate
    for (int i = 0; i < N; i++){
        func_result = pthread_join(clients_id[i],NULL);
        if (func_result != 0)
            error("pthread join");   
    }

    //give barber time to shave all clients
    sleep(5);
    //cancel barber thread
    func_result = pthread_cancel(barber_id);
    if(func_result != 0)
        error("pthread cancel");
    
    printf("Koniec klientów na dziś, golibroda może spać\n");
    
    return 0;
}