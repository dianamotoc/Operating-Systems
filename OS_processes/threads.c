#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

typedef struct
{
    int id;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
} TH_STRUCT;

typedef struct
{
    int value;
    sem_t *logSem;
} TH_STRUCT_BARRIER;

typedef struct
{
    int value;
} TH_STRUCT_PROCESSES;

int turn3;  //begin 3
int turn1;  //end 1
int count = 0;

sem_t* blockTh1 = NULL;
sem_t *blockTh2 = NULL;

void* thread_function_processes(void* arg)
{
    TH_STRUCT_PROCESSES *argument = (TH_STRUCT_PROCESSES*) arg;

    if(argument->value == 3)
        sem_wait(blockTh2);
    info(BEGIN, 6, argument->value);
    info(END, 6, argument->value);
    if (argument->value == 4)
        sem_post(blockTh1);
    return NULL;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
sem_t intermediar;

int endT11;
int isT11;
int terminat;
void* thread_function_barrier(void* arg)
{
    TH_STRUCT_BARRIER *argument = (TH_STRUCT_BARRIER*) arg;
    sem_wait(argument->logSem);

    info(BEGIN, 5, argument->value);
  /*  count++;
    if(argument->value == 11)
    {
        isT11 = 1;
    }
    if(count <= 4)
    {
        if(isT11 == 1)
        {

        }
    }
    if(terminat == 1)
        sem_post(&intermediar);
    else
    if(count == 5){
        if(isT11 == 0)
        {
            sem_post(&intermediar);
        }
        if(isT11 == 1)
        {
            terminat = 1;
            //sem_post(&intermediar);

        }
    }
    if((argument->value != 11))
        sem_wait(&intermediar);

    if(argument->value == 11){
        info(END, 5, 11);
        sem_post(&intermediar);
        sem_post(&intermediar);
        sem_post(&intermediar);
        sem_post(&intermediar);
        //sem_post(&intermediar);
    }
    else*/
        info(END, 5, argument->value);

    count--;

    sem_post(argument->logSem);
    return NULL;
}

void* thread_function(void* arg)
{
    TH_STRUCT *argument = (TH_STRUCT*) arg;
    pthread_mutex_lock(argument->lock);

    if(argument->id == 1)
    {
        while(turn3 != 1)
        {
            pthread_cond_wait(argument->cond, argument->lock);
        }
        info(BEGIN, 2, argument->id);
        info(END, 2, argument->id);

        turn1 = 1;
        pthread_cond_signal(argument->cond);
    }
    else
    {
        if(argument->id == 3)
        {
            turn3 = 1;
            info(BEGIN, 2, argument->id);
            pthread_cond_signal(argument->cond);
            while(turn1 != 1)
            {
                pthread_cond_wait(argument->cond, argument->lock);
            }
            info(END, 2, argument->id);
        }
        else
        {
            if (argument->id == 2)
            {
                sem_wait(blockTh1);
            }


            info(BEGIN, 2, argument->id);
            info(END, 2, argument->id);

            if(argument->id == 2)
            {
                sem_post(blockTh2);
            }
        }

    }
    pthread_mutex_unlock(argument->lock);
    return NULL;
}


//void* meeting_point(void* arg)
//{
////varianta curs
//    TH_STRUCT_SEM *argument = (TH_STRUCT_SEM*) arg;
//
//    pthread_mutex_lock(&mutex_temp1);
//    info(BEGIN, 5, argument->value);
//    count++;
//
//    if(count == 5)
//    {
//        sem_wait(argument->bar2);
//        sem_post(argument->bar1);
//    }
//
//    pthread_mutex_unlock(&mutex_temp1);
//    info(END, 5, argument->value);
//    sem_wait(argument->bar1);
//    sem_post(argument->bar1);
//
//    pthread_mutex_lock(&mutex_temp2);
//
//    count--;
//    if(count == 0)
//    {
//        sem_wait(argument->bar1);
//        sem_post(argument->bar2);
//    }
//    //info(END, 5, argument->value);
//    pthread_mutex_unlock(&mutex_temp2);
//
//    sem_wait(argument->bar2);
//    sem_post(argument->bar2);
//
//
//
//
//
////var 3 carte
//    TH_STRUCT_SEM *argument = (TH_STRUCT_SEM*) arg;
//    sem_wait(argument->logSem);
//    count++;
//    if (count == 5)
//    {
//        sem_post(argument->bar1);
//        sem_post(argument->bar1);
//        sem_post(argument->bar1);
//        sem_post(argument->bar1);
//        sem_post(argument->bar1);
//    }
//    sem_post(argument->logSem);
//
//    sem_wait(argument->bar1);
//
//    info(BEGIN, 5, argument->value);
//    info(END, 5, argument->value);
//
//    sem_wait(argument->logSem);
//    count--;
//    if (count == 0)
//    {
//        sem_post(argument->bar2);
//        sem_post(argument->bar2);
//        sem_post(argument->bar2);
//        sem_post(argument->bar2);
//        sem_post(argument->bar2);
//    }
//    sem_post(argument->logSem);
//
//    sem_wait(argument->bar2);
//
//    return NULL;
//}


int main()
{
    pid_t p2, p3, p4, p5, p6, p7, p8, p9;
    init();
    sem_unlink("SemTh1");

    pthread_t tidsMEU[4];

    blockTh1 = sem_open("SemTh1", O_CREAT, 0644, 0);
    if( blockTh1 == NULL)
    {
        perror("Could not aquire the semaphore");
        return 0;
    }
    sem_unlink("SemTh2");
    blockTh2 = sem_open("SemTh2", O_CREAT, 0644, 0);
    if( blockTh2 == NULL)
    {
        perror("Could not aquire the semaphore");
        return 0;
    }


    info(BEGIN, 1, 0);    // begin p1
    p2 = fork();
    if (p2 == -1)
    {
        perror("Could not create child process");
        return -1;
    }
    else if(p2 == 0)
    {
        info(BEGIN, 2, 0);   // begin p2
        TH_STRUCT params[4];
        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

        int i;
        for(i = 0; i < 4; i++)
        {
            params[i].id = i + 1;
            params[i].lock = &lock;
            params[i].cond = &cond;
            pthread_create(&tidsMEU[i], NULL, thread_function, &params[i]);
        }

        p3 = fork();
        if (p3 == -1)
        {
            perror("Could not create child process");
            return -1;
        }

        else if(p3 == 0)
        {
            info(BEGIN, 3, 0);   // begin p3
            p4 = fork();
            if (p4 == -1)
            {
                perror("Could not create child process");
                return -1;
            }
            else if(p4 == 0)
            {
                info(BEGIN, 4, 0);
                info(END, 4, 0);
            }
            else
            {
                p5 = fork();
                if(p5 == -1)
                {
                    perror("Could not create child process");
                    return -1;
                }
                else if(p5 == 0)
                {
                    info(BEGIN, 5, 0);
                    sem_t logSem;
                    int i;
                    TH_STRUCT_BARRIER params[50];
                    pthread_t tids[50];

                    if(sem_init(&logSem, 0, 5) != 0)   //aici e semaforul cu 5 permisiuni
                    {
                        perror("Could not init the semaphore");
                        return -1;
                    }

                     if(sem_init(&intermediar, 0, 0) != 0)   //aici e semaforul cu 5 permisiuni
                    {
                        perror("Could not init the semaphore");
                        return -1;
                    }

                    printf("\n");
                    for(i = 0; i < 50; i++)
                    {
                        params[i].value = i + 1;
                        params[i].logSem = &logSem;
                        pthread_create(&tids[i], NULL, thread_function_barrier, &params[i]);
                    }

                    for(i = 0; i < 50; i++)
                        pthread_join(tids[i], NULL);

                    printf("\n");
                    sem_destroy(&logSem);

                    p6 = fork();
                    if(p6 == -1)
                    {
                        perror("Could not create child process");
                        return -1;
                    }
                    else if(p6 == 0)
                    {
                        info(BEGIN, 6, 0);

                        int i;
                        TH_STRUCT_PROCESSES params[5];
                        pthread_t tids[5];


                        printf("\n");
                        for(i = 0; i < 5; i++)
                        {
                            params[i].value = i + 1;
                            pthread_create(&tids[i], NULL, thread_function_processes, &params[i]);
                        }

                        for(i = 0; i < 5; i++)
                            pthread_join(tids[i], NULL);

                        printf("\n");


                        p7 = fork();
                        if(p7 == -1)
                        {
                            perror("Could not create child process");
                            return -1;
                        }
                        else if(p7 == 0)
                        {
                            info(BEGIN, 7, 0);
                            info(END, 7, 0);
                        }
                        else
                        {
                            wait(NULL);
                            info(END, 6, 0);
                        }
                    }
                    else
                    {
                        wait(NULL);
                        info(END, 5, 0);
                    }
                }
                else
                {
                    p9 = fork();
                    if(p9 == -1)
                    {
                        perror("Could not create child process");
                        return -1;
                    }
                    else if(p9 == 0)
                    {
                        info(BEGIN, 9, 0);
                        info(END, 9, 0);
                    }
                    else
                    {
                        wait(NULL);
                        wait(NULL);
                        wait(NULL);
                        info(END, 3, 0);
                    }
                }
            }
        }
        else
        {
            wait(NULL);
            p8 = fork();
            if(p8 == -1)
            {
                perror("Could not create child process");
                return -1;
            }
            else if(p8 == 0)
            {
                info(BEGIN, 8, 0);
                info(END, 8, 0);
            }
            else
            {


                wait(NULL);
                wait(NULL);


                for(i = 0; i < 4; i++)
                    pthread_join(tidsMEU[i], NULL);

                info(END, 2, 0);
            }
        }
    }
    else
    {
        wait(NULL);
        info(END, 1, 0);
    }
    return 0;
}
