/** Compilation: gcc -o memreader memreader.c -lrt -lpthread **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "shmem.h"

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}

int main() {
  int fd = shm_open(BackingFile, O_RDWR, AccessPerms);  /* empty to begin */
  if (fd < 0) report_and_exit("Can't get file descriptor...");

  /* get a pointer to memory */
  caddr_t memptr = mmap(NULL,       /* let system pick where to put segment */
                        BUFFER_SIZE,   /* how many bytes */
                        PROT_READ | PROT_WRITE, /* access protections */
                        MAP_SHARED, /* mapping visible to other processes */
                        fd,         /* file descriptor */
                        0);         /* offset: start at 1st byte */
  if ((caddr_t) -1 == memptr) report_and_exit("Can't access segment...");

  /* create a semaphore for mutual exclusion */
  sem_t* semptr = sem_open(SemaphoreName, /* name */
                           O_CREAT,       /* create the semaphore */
                           AccessPerms,   /* protection perms */
                           0);            /* initial value */
  if (semptr == (void*) -1) report_and_exit("sem_open");

  while(1) {
    int val = 0;
    sem_getvalue(semptr, &val);
    printf("semval= %d\n", val);
    /* use semaphore as a mutex (lock) by waiting for writer to increment it */
    printf("waiting semaphore...\n");
    if (!sem_wait(semptr)) { /* wait until semaphore != 0 */
      printf("Waiting DONE.\n");
      printf("%s\n", memptr);

      sem_post(semptr);
      if(strncmp(memptr, "exit", strlen("exit")) == 0) {
        /* cleanup */
        munmap(memptr, BUFFER_SIZE);
        close(fd);
        //sem_unlink(SemaphoreName);
        sem_close(semptr);
        //unlink(BackingFile);
        printf("exit...\n");
        break;
      }
      sleep(1);
    }
  }

  
  exit(0);
}