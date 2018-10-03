/*
Programmer: Briton A. Powe          Program Homework Assignment #2
Date: 9/23/18                       Class: Operating Systems
File: Master.c
------------------------------------------------------------------------
Program Description:
Takes in integer command line parameters for option -n and -s to fork
and execvp n number of processes, allowing only s to be run simultaneously.
Program terminates children and self after catching a signal from a 
2 sec alarm or user enters Ctrl-C. Before ending, program outputs integer
values in shared memory and deallocates the shared memory segment. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <signal.h>
#include <semaphore.h>

//Structure to be used in shared memory.
struct clock {
    int sec;
    int millisec;
    char shmMsg[50];
};

struct clock *clockptr;

void sigQuitHandler(int);

int main(int argc, char *argv[]){ 
    signal(SIGQUIT, sigQuitHandler);
    int shmid, i;
    key_t key = 3670400;
    sem_t *mutex;

    //Finding shared memory segment.
    if ((shmid = shmget(key, sizeof(struct clock), 0666|IPC_CREAT)) < 0) {
        perror(strcat(argv[0],": Error: Failed shmget find"));
        exit(-1);
    }

    //Attaching to memory segment.
    if ((clockptr = shmat(shmid, NULL, 0)) == (void *) -1) {
        perror(strcat(argv[0],": Error: Failed shmat attach"));
        exit(-1);
    }
    
    do {
        sem_wait (mutex);           /* P operation */
        fprintf(stderr,"  Child(%ld) is in critical section.\n", (long)getpid());
        if ((strcmp(clockptr->shmMsg, "")) == 0){
            sprintf(clockptr->shmMsg, "%ld : The time is %d.%d", (long)getpid(), clockptr->sec, clockptr->millisec);
            sem_post(mutex);
            break;
        }
        fprintf(stderr, "Clock: %d.%d \nMessage: %s\n", clockptr->sec, clockptr->millisec, clockptr->shmMsg);
        fprintf(stderr, "Leaving critical!\n");
        sem_post (mutex);           /* V operation */
    } while(1);

    //Unlinking from semaphore.
    sem_unlink("ossSem");

    //Detaching from memory segment.
    if (shmdt(clockptr) == -1) {
      perror(strcat(argv[0],": Error: Failed shmdt detach"));
      clockptr = NULL;
      exit(-1);
   }

    return 0;
}

//Handler for quit signal.
void sigQuitHandler(int sig) {
   abort();
}