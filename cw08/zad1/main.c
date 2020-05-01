#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define M 256
#define BUFF_SIZE 70

int histogram[M];
int width, height, max_val;


typedef struct thread_info
{
    int **picture;//pointer to matrix containing picture
    int histogram[M];//histogram of thread to avoid race codition, it's acceptable when gray scale is ralatively small and there aren't much threads
    //block mode
    int start_idx;//starting index of slice counted by thread
    int end_idx;//end index of slice counted by thread
    //interleaved mode
    int step;//length of step in interleaved mode
    //sign mode
    int min;//minal value of gray scale counted by thread
    int max;//maximal value of gray scale counted by thread

} thread_info;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int **read_file(char *file_name)
{
    char ch;
    int val;

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
        error("Can't open file");

    ch = getc(fp);
    if (ch != 'P')
        error("Wrong format of file");

    ch = getc(fp);
    if (ch != '2')
        error("Wrong format of file");

    while (getc(fp) != '\n'); //skip to the end of line

    while (getc(fp) == '#') //skip comments
    {
        while (getc(fp) != '\n');
    }

    fseek(fp, -1, SEEK_CUR);

    fscanf(fp, "%d %d", &width, &height);
    fscanf(fp, "%d", &max_val);

    printf("%d  %d\n", width, height);

    int **picture = (int **) calloc(height, sizeof(int *));
    for(int i = 0; i < height; i++){
        picture[i] = (int *) calloc(width, sizeof(int));
    }


    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            fscanf(fp, "%d", &val);
            picture[row][col] = val;
        }
    }

    fclose(fp);

    return picture;
}

struct timespec *time_diff(struct timespec start, struct timespec end)
{
    struct timespec *diff = (struct timespec *) calloc(1, sizeof(struct timespec));
    diff->tv_sec = end.tv_sec - start.tv_sec;
    diff->tv_nsec = end.tv_nsec - start.tv_nsec;

    if (start.tv_nsec > end.tv_nsec)
    {
        diff->tv_sec--;
        diff->tv_nsec = -diff->tv_nsec;
    }

    return diff;
}

void *thread_block(void *info)
{
    thread_info *t_info = (thread_info *)info;

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    for (int row = 0; row < height; row++)
    {
        for (int col = t_info->start_idx; col < t_info->end_idx; col++)
        {
            t_info->histogram[t_info->picture[row][col]]++;
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    
    struct timespec *tm = time_diff(start, end);
    
    pthread_exit((void *)tm);
}


void *thread_interleaved(void *info){
    thread_info *t_info = (thread_info *)info;

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);


    for (int row = 0; row < height; row++)
    {
        for (int col = t_info->start_idx; col < width; col+=t_info->step)
        {
            t_info->histogram[t_info->picture[row][col]]++;
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    struct timespec *tm = time_diff(start, end);
    
    pthread_exit((void *)tm);
}

void *thread_sign(void *info){
    thread_info *t_info = (thread_info *)info;

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);


    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            //if this pixel is in range counted by this thread incremetnt histogram
            if(t_info->picture[row][col] >= t_info->min && t_info->picture[row][col] < t_info->max)
                t_info->histogram[t_info->picture[row][col]]++;
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    struct timespec *tm = time_diff(start, end);
    
    pthread_exit((void *)tm);
}


void log_to_file(char *file_name, char *mode, int threads_num, struct timespec *total_time, pthread_t *threads, struct timespec **times){
    FILE *fp = fopen(file_name, "a");
    if(fp == NULL)
        error("fopen log file");

    fprintf(fp, "\n####### %s -> %d threads #######\n", mode, threads_num);
    fprintf(fp, "total time: %lds %ldms\n", total_time->tv_sec, total_time->tv_nsec/1000);

    for(int i = 0; i < threads_num; i++){
        fprintf(fp, "thread %ld: %lds %ldms\n", threads[i], times[i]->tv_sec, times[i]->tv_nsec/1000);
    }

    fclose(fp);
}

void save_histogram(char *file_name, int *histogram){
    FILE *fp = fopen(file_name, "w");

    fprintf(fp, "gray scale - count\n");

    for (int i = 0; i < M; i++)
    {
        fprintf(fp, "%d - %d\n", i, histogram[i]);    
    }
    
    fclose(fp);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Wrong number of arguments!\n");
        return 1;
    }

    int threads_num = atoi(argv[1]);
    char *mode = argv[2];
    char *input_file_name = argv[3];
    char *result_file_name = argv[4];

    int **picture = read_file(input_file_name);


    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    thread_info **infos = (thread_info **)calloc(threads_num, sizeof(thread_info *));
    

    pthread_t *threads = (pthread_t *)calloc(threads_num, sizeof(pthread_t));

    if (strcmp(mode, "block") == 0)
    {
        int step = width / threads_num;

        for (int i = 0; i < threads_num; i++)
        {
            infos[i] = (thread_info *)calloc(1, sizeof(thread_info));
            infos[i]->start_idx = i * step;
            infos[i]->end_idx = (i + 1) * step;
            infos[i]->picture = picture;
        }

        for (int i = 0; i < threads_num; i++)
        {
            if (pthread_create(&threads[i], NULL, thread_block, (void *)infos[i]) != 0)
                error("pthread_create");
        }
    }
    else if (strcmp(mode, "interleaved") == 0)
    {
        
        for (int i = 0; i < threads_num; i++)
        {
            infos[i] = (thread_info *)calloc(1, sizeof(thread_info));
            infos[i]->start_idx = i ;
            infos[i]->step = threads_num;
            infos[i]->picture = picture;
        }

        for (int i = 0; i < threads_num; i++)
        {
            if (pthread_create(&threads[i], NULL, thread_interleaved, (void *)infos[i]) != 0)
                error("pthread_create");
        }
    }
    else if (strcmp(mode, "sign") == 0){
        int range = M / threads_num;

        for (int i = 0; i < threads_num; i++)
        {
            infos[i] = (thread_info *)calloc(1, sizeof(thread_info));
            infos[i]->min = i * range;
            infos[i]->max = (i + 1) * range;
            infos[i]->picture = picture;
        }

        for (int i = 0; i < threads_num; i++)
        {
            if (pthread_create(&threads[i], NULL, thread_sign, (void *)infos[i]) != 0)
                error("pthread_create");
        }
    }
    else{
        fprintf(stderr,"No such mode\n");
    }

    struct timespec **times = (struct timespec **) calloc(threads_num, sizeof(struct timespec *));


    void *retval;

    for (int i = 0; i < threads_num; i++)
    {
        pthread_join(threads[i], &retval);
        
        struct timespec *tm = (struct timespec *) retval;
        printf("%ld: %lds %ldms\n", threads[i], tm->tv_sec, tm->tv_nsec / 1000);

        //add the thread result to global result
        for (int j = 0; j < M; j++)
        {

            histogram[j] += infos[i]->histogram[j];
        }

        times[i] = tm;
        free(infos[i]);
    }
    
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    struct timespec *total_time = time_diff(start, end);

    log_to_file("Times.txt", mode, threads_num, total_time, threads, times);

    save_histogram(result_file_name, histogram);

    for(int i = 0; i < threads_num; i++){
        free(times[i]);
    }
    free(times);

    free(total_time);

    for(int i = 0; i < height; i++){
        free(picture[i]);
    }
    free(picture);

    free(threads);
    free(infos);

    return 0;
}