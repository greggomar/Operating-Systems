/* Gregory O'Marah - netID: gdo
   U3424-0613
   COP 4600 Operating Systems
   project 2 - semaphores
                              */
/*ass2*/

#include <stdio.h>          // printf() 
#include <sys/types.h>      // key_t, sem_t, pid_t 
#include <sys/shm.h>        // shmat(), IPC_RMID   
#include <errno.h>          // errno, ECHILD  
#include <semaphore.h>      // sem_opensem_destroy(), sem_wait()..
#include <fcntl.h>          // O_CREAT, O_EXEC 
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h> 
#include  <sys/sem.h>
#include  <sys/wait.h>
   // shared memory key
#define SHMKEY ((key_t) 300L)
   // semaphore key
#define SEMKEY ((key_t) 400L)
// number of semaphores being created
#define NSEMS 1

void Processx(int i); 

typedef struct // struct contains vars for sharing between Processes p1, p2, p3
{
    int count;
    int value, value1;            /*      shared variable         *//*shared */

} shared_mem;

shared_mem *total; // declare var * (total) for shared memory between p1, p2

int sem_id;// semaphore id

// semaphore buffers
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};
struct sembuf *P =&OP;
struct sembuf *V =&OV;

// semapore union used to generate semaphore
typedef union{
  int val;    
  struct semid_ds *buf;
  ushort *array;
} semunion;

// Wait() function for semaphore
int POP()
{ 
  int status;
  status = semop(sem_id, P,1);
  return status;
}

// Signal() function for semaphore
int VOP()
{ 
  int status;
  status = semop(sem_id, V,1);
  return status;
}

int main (){
    int i;                        //      loop variables          
    key_t j;                      //      shared memory key       
    int shmid;                    //      shared memory id        
    pid_t pid, pid1, pid2, pid3;  //      fork pid                
    unsigned int n = 3;           //      fork count              
    
    char *shmadd;
    shmadd = (char *) 0;

    /* initialize a shared variable in shared memory */
//    shmkey = ftok ("/dev/null", 5);       /* valid directory name and a number */
    shmid = shmget (SHMKEY, sizeof (int), IPC_CREAT | 0666);
    j = i = n; // placeholder variables
//    printf("shmkey: %d\n", shmkey);
//    printf("SHMKEY: %d\n", SHMKEY);

    if (shmid < 0){                           /* shared memory error check */
        perror ("shmget\n");
        exit (1);
    }

   // attach total to shared mem segment
   if ((total = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1)
   {
      perror ("shmat");
      exit (0);
   }
 
    total->count = 0; // initialize total->count to zero
    total->value = 0;
    /********************************************************/

  int status;
  semunion semctl_arg;
  semctl_arg.val = 1;

/* Create semaphores */
  sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
  if(sem_id < 0) printf("Error in creating the semaphore./n");

  /* Initialize semaphore */
  total->value1 =semctl(sem_id, 0, SETVAL, semctl_arg);
  total->value =semctl(sem_id, 0, GETVAL, semctl_arg);
  if (total->value < 1) printf("Eror detected in SETVAL.\n");

   if ((pid1 = fork()) == 0)
      Processx(1);
   
   if ((pid1 != 0) && (pid2 = fork()) == 0) // fork child 2, Process 2
      Processx(2);
      
   if ((pid1 != 0) && (pid2 != 0) && (pid3 = fork()) == 0) // fork child 3, Process 3
      Processx(3);
   
   if ((pid1 != 0) && (pid2 != 0) && (pid3 != 0))
   // Parent process
    {
        // wait for child processes to exit
        while ((pid = (waitpid (pid, NULL, 0))) ){
          pid1 = ((pid > pid1)? pid: pid1); 
          if (errno == ECHILD)
            break;
        }
        printf ("Child with ID %d has exited.\n", pid1);

        while ((pid2 = (waitpid (pid, NULL, 0))) ){
          pid2 = ((pid > pid2)? pid: pid2); 
          if (errno == ECHILD)
            break;
        }
        printf ("Child with ID %d has exited.\n", pid2);              
      
        while ((pid3 = (waitpid (pid, NULL, 0))) ){
          pid3 = ((pid > pid3)? pid: pid3); 
          if (errno == ECHILD)
            break;
        }
        printf ("Child with ID %d has exited.\n", pid3);
        printf ("\n\tEnd of  Simulation.\n\n");

        /* shared memory detach */
        shmdt (total);
      // free shared memory
      if ((shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0)) == -1)
      {
         perror ("shmctl");
         exit (-1);
      }

      /* De-allocate semaphore */
      semctl_arg.val = 0;
      status =semctl(sem_id, 0, IPC_RMID, semctl_arg);
      if( status < 0) printf("Error in removing the semaphore.\n");
    }
    return 0;
}
    // child process functionality
void Processx(int i){
        switch (i){
            case 1: 
            {
               int k = 0; // declare and initialize counting vars
               while (k < 100000) // k will go up to 100000
               {                      
                  POP(); 
                  total->count++;       // increment the total->value (critical shared mem access)
                  k++;
                  VOP(); 
               }
               printf ("\n\nFrom Process 1: counter = %d.\n", total->count); // p1 done, outputs to screen
               return;
            }
            case 2:
            {
               int k = 0; // declare and initialize counting vars
               while (k < 200000) // k will go up to 200000
               {                      
                  POP(); 
                  total->count++;       // increment the total->value (critical shared mem access)
                  k++;
                  VOP(); 
               }
               printf ("From Process 2: counter = %d.\n", total->count);   // p2 finished, output
               return;
            }
            case 3: 
            {
               int k = 0; // declare and initialize counting vars
               while (k < 300000) // k will go up to 300000
               {                      
                  POP(); 
                  total->count++;       // increment the total->value (critical shared mem access)
                  k++;
                  VOP(); 
               }
               printf ("From Process 3: counter = %d.\n", total->count);   // p2 finished, output
               return;
            }
            default:
               break;
        }
        return;
    }