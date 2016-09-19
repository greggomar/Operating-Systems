/* Gregory O'Marah - netID: gdo
   U3424-0613
   COP 4600 Operating Systems
   project 1 - synchronization
                              */
/*ass1*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>

/* change the key number */
#define SHMKEY ((key_t) 6545)

typedef struct // struct contains values for sharing between processes p1, p2
{
   int value, turn, first, second;
   int flag[2];

} shared_mem;

shared_mem *total; // declare var * (total) for shared memory between p1, p2


/*----------------------------------------------------------------------*
 * This function increases the value of shared variable "total"
 *  by one all the way to 100000
 *----------------------------------------------------------------------*/

void process1 () // p1 : process 1
{
   int k = 0, interrupt  = 0; // declare and initialize counting vars
   while (k < 100000) // k will go up to 100000
   {
      total->flag[0] = 1; // begin peterson's solution : set flag 0 = 1 (true)
      total->turn = 2;    // (process 2's turn)
      
      while (total->flag[1] && total->turn == 2) // *** Waiting section ***
         total->first = 1;  // first is set to indicate that P2 interrupted P1
                           // end of waiting section
                           // *****start the critical section
      if (total->second == 1)  // check to see if P1 interrupted P2
         interrupt++;         // increment counter if interrupt
      k++;
      total->value++;       // increment the value (critical shared mem access)
      total->first = 0;    // set first to 0 so we indicate not waiting
      total->flag[0] = 0; // end critical section, flag 0 set to 0 (false)
   }
   printf ("\nFrom process 1: total = %d", total->value); // p1 done, outputs to screen
   printf ("\nProcess 2 interrupted %d times in critical section by Process 1.", interrupt);
}


/*----------------------------------------------------------------------*
 * This function increases the vlaue of shared memory variable "total"
 *  by one all the way to 100000
 *----------------------------------------------------------------------*/

void process2 () // p2 : process 2
{
   int k = 0, interrupt  = 0; // declare and initialize counting vars
   while (k < 100000) // k will go up to 100000
   {
      total->flag[1] = 1;// begin peterson's solution : set flag 1 = 1 (true)
      total->turn = 1;  // (process 1's turn)
      
      while (total->flag[0] && total->turn == 1) // *** Waiting Section ***
         total->second = 1; // second set to indicate that P1 interrupted P2
                           // end of waiting section
                           // *****start the critical section
      
      if (total->first == 1) // check to see if P2 interrupted P1
         interrupt++;         // increment counter if interrupt
      
      k++;
      total->value++;       // increment the value (critical shared mem access)
      total->second = 0;    // set first to 0 so we indicate not waiting
      total->flag[1] = 0;   // end critical section, flag 1 set to 0 (false)
   }
   printf ("\nFrom process 2: total = %d", total->value);   // p2 finished, output
   printf ("\nProcess 1 interrupted %d times in critical section by Process 2.", interrupt);
}



/*----------------------------------------------------------------------*
 * MAIN()
 *----------------------------------------------------------------------*/

int main()
{
   
   int   shmid;
   int   pid1;
   int   pid2;
   int   ID;
   int	status;
   char *shmadd;
   shmadd = (char *) 0;
   
   /* Create and connect to a shared memory segmentt*/
   
   if ((shmid = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
   {
      perror ("shmget");
      exit (1);
   }
   
   // attach total to shared mem segment
   if ((total = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1)
   {
      perror ("shmat");
      exit (0);
   }
   
   total->value = 0; // initialize total-value to zero
   
   if ((pid1 = fork()) == 0)
   {
      printf ("\nFork child process 1\n"); //  fork child 1, process 1
      process1();
   }
   
   if ((pid1 != 0) && (pid2 = fork()) == 0) // fork child 2, process 2
   {
      printf ("\nFork child process 2\n");
      process2();
   }
   
   if ((pid1 != 0) && (pid2 != 0))
   {
      // get pids and wait for children to exit
      int finished = wait(&status);
      ID = finished;
      finished = wait(&status);
      // print out child status on exit
      printf("\n\nChild with ID %d has just exited.\r\n", (pid1 == finished)?pid1:ID );
      printf("Child with ID %d has just exited.\r\n", (pid2 == ID)?pid2:finished );
      
      // free shared memory
      if ((shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0)) == -1)
      {
         perror ("shmctl");
         exit (-1);
      }
      
      // end of program, termination
      printf ("\t\t  End of Program.\n\n");
   }
   return 0;

}

