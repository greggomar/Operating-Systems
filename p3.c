// Gregory O'Marah - U34240613 
// Operating Systems - Professor Korzhova 
// Spring 2016 - Project 3 - Shared memory buffer

#define _REENTRANT
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

// Using shared memory for buffer
#define SHMKEY ((key_t) 9870)
// Buffer size 15, circular
#define BUFFER_SIZE 15
// The buffer should be treated as circular buffer
#define BUFFER_WRAP(x) x%BUFFER_SIZE

// structs
typedef struct{ char* value; } shared_mem;  //shared_mem (memory)
typedef struct{ int value; } shared_dat;    //shared_data (data)

// require 3 semaphores;
sem_t empty;
sem_t full;
sem_t mutex;
int start = 0;
int end = 0;
shared_mem *buffer;
char newChar;
FILE* fp;

shared_dat *counter;

// prototypes
void* FuncProd();
void* FuncCons();

// main program
int main()
{

    fp = fopen("mytest.dat", "r");

    //*****************init everything: threads, sems, shmem********///
    int r=0;
    int shmid;    // shared memory ID //
    pthread_t producer[1];     // process id for thread 1 //
    pthread_t consumer[1];     // process id for thread 2 //
    pthread_attr_t attr;     // attribute pointer array //
    char *shmadd;
    shmadd = (char *) 0;

    //create shared memory seg, if return -1 then print error
    if ((shmid = shmget (SHMKEY, sizeof(char), IPC_CREAT | 0666)) < 0)
    {
        perror ("shmget");
        return (1);
    }

    //connect process to shared memory segment.  If return is -1 then print error
    if ((buffer = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1)
    {
        perror ("shmat");
        return (0);
    }

    //set the 
    char buffer_array[15];
    buffer->value = buffer_array;

    counter = (shared_dat *) malloc(sizeof(shared_dat));

    // initialize shared memory to 0 //
    counter->value = 0 ;

    // initialize semaphores
    sem_init(&empty,0,BUFFER_SIZE);
    sem_init(&full,0,0);
    sem_init(&mutex,0,1);

    // flush std out
    fflush(stdout);
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);  // system-wide contention // 

    // Create the threads //
    pthread_create(&producer[0], &attr, FuncProd, 0);
    pthread_create(&consumer[0], &attr, FuncCons, 0);

    // Wait for the threads to finish //
    pthread_join(producer[0], 0);
    pthread_join(consumer[0], 0);

    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);

    //release shared mem with IPC_RMID and print "end"
    if ((shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0)) == -1)
    {
        perror ("shmctl");
        exit (-1);
    }
    fclose(fp);

    //**************once threads finished and * reached in buffer**********///
    printf("from parent counter  =  %d\n", counter->value);
    printf("---------------------------------------------------------------------------\n");
    printf("\t\t    End of simulation\n");

    return 0;    
}

void* FuncProd()
{
    bool finished = false;
    while(!finished)
    {
        sem_wait(&empty);
        sem_wait(&mutex);
        end++;

        if(fscanf(fp,"%c",&newChar) != EOF)
        {
            buffer->value[BUFFER_WRAP(end)] = newChar;
            //printf("%c",newChar);
        }
        else
        {
            buffer->value[BUFFER_WRAP(end)] = '*';
            finished = true;
        }
        sem_post(&full);        
        sem_post(&mutex);
    }
    return (void*)finished;
}

void* FuncCons()
{
    bool finished = false;
    char val;
    while(!finished)
    {
        sem_wait(&full);
        sem_wait(&mutex);
        start++;

        // 1 sec. sleep
        sleep(1);
        if((val = buffer->value[BUFFER_WRAP(start)]) != '*')
        {
            printf("\nConsuming: %c",val);
            counter->value++;
        }
        else
        {
            finished = true;
        }
        sem_post(&mutex);
        sem_post(&empty);
    }
    return (void*)finished;
}


