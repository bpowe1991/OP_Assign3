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
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <signal.h>
#include <semaphore.h>

//Structure to be used in shared memory.
struct clock {
    int sec;
    int nanoSec;
    char shmMsg[50];
    pid_t child;
};

struct clock *clockptr;

void sigQuitHandler(int);

int main(int argc, char *argv[]){ 
    signal(SIGQUIT, sigQuitHandler);
    int shmid, deadlineSec, deadlineNanoSec, randInt;
    key_t key = 3670400;
    sem_t *mutex;

    srand(time(0));

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

    if ((mutex = sem_open ("ossSem", O_CREAT, 0644, 1)) == NULL){
        perror(strcat(argv[0],": Error: Failed semaphore creation"));
        exit(-1);    
    } 
    
    deadlineSec = clockptr->sec;
    deadlineNanoSec = clockptr->nanoSec;
    randInt = (rand() % (1000000)) + 1;
    deadlineNanoSec += randInt;

    if (deadlineNanoSec > ((int)1e9)) {
        deadlineSec += (deadlineNanoSec/((int)1e9));
        deadlineNanoSec = (deadlineNanoSec%((int)1e9));
    }

    fprintf(stderr, "Child %ld - %d.%d\n", (long)getpid(), deadlineSec, deadlineNanoSec);

    do {
        sem_wait (mutex);          
        if ((clockptr->sec > deadlineSec) || 
            (clockptr->sec == deadlineSec && clockptr->nanoSec >= deadlineNanoSec)){
            if ((strcmp(clockptr->shmMsg, "")) == 0){
                sprintf(clockptr->shmMsg, "Child %ld : %d.%d time reached", 
                        (long)getpid(), clockptr->sec, clockptr->nanoSec);
                clockptr->child = getpid();
                sem_post(mutex);
                break;
            }
        }
        sem_post (mutex); 
    } while(1);

    //Unlinking from semaphore.
    sem_close(mutex);

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