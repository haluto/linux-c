/** Compilation: gcc -o memwriter memwriter.c -lrt -lpthread **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "shmem.h"

static void *update_shm_main(void * p);
static void clean_up(void);
static void sigcb(int signo);

typedef struct {
  int fd;
  caddr_t memptr;
  sem_t * semptr;
} static_obj_t;

static static_obj_t s_obj;

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}

int main() {
  memset(&s_obj, 0, sizeof(s_obj));
  signal(SIGHUP, sigcb);
  signal(SIGINT, sigcb);
  signal(SIGQUIT, sigcb);
  signal(SIGTERM, sigcb);

  int fd = shm_open(BackingFile,      /* name from smem.h */
                    O_RDWR | O_CREAT, /* read/write, create if needed */
                    AccessPerms);     /* access permissions (0644) */
  if (fd < 0) report_and_exit("Can't open shared mem segment...");

  ftruncate(fd, ByteSize); /* get the bytes */

  caddr_t memptr = mmap(NULL,       /* let system pick where to put segment */
                        ByteSize,   /* how many bytes */
                        PROT_READ | PROT_WRITE, /* access protections */
                        MAP_SHARED, /* mapping visible to other processes */
                        fd,         /* file descriptor */
                        0);         /* offset: start at 1st byte */
  if ((caddr_t) -1  == memptr) report_and_exit("Can't get segment...");

  fprintf(stderr, "shared mem address: %p [0..%d]\n", memptr, ByteSize - 1);
  fprintf(stderr, "backing file:       /dev/shm/%s\n", BackingFile );

  /* semaphore code to lock the shared mem */
  sem_t* semptr = sem_open(SemaphoreName, /* name */
                           O_CREAT,       /* create the semaphore */
                           AccessPerms,   /* protection perms */
                           0);            /* initial value */
  if (semptr == (void*) -1) report_and_exit("sem_open");

  strcpy(memptr, MemContents); /* copy some ASCII bytes to the segment */

  /* increment the semaphore so that memreader can read */
  if (sem_post(semptr) < 0) report_and_exit("sem_post");

  pthread_t tid = 0;
  pthread_create(&tid, NULL, &update_shm_main, NULL);

  s_obj.fd = fd;
  s_obj.memptr = memptr;
  s_obj.semptr = semptr;

  while(1) {
    sleep(10);
  }

  clean_up();
  return 0;
}

static void *update_shm_main(void *p)
{
  char buffer[ByteSize] = {0};
  while(1) {
    printf("Enter data: ");
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, ByteSize, stdin);
    if (!sem_wait(s_obj.semptr)) { /* wait until semaphore != 0 */
      memcpy(s_obj.memptr, buffer, ByteSize);
      if (sem_post(s_obj.semptr) < 0) report_and_exit("sem_post");

      if(strncmp(buffer, "exit", strlen("exit")) == 0) {
        sleep(2);//waiting the reader stopped.
        clean_up();
        exit(0);
      }
    }
  }
}


static void clean_up(void)
{
  /* clean up */
  printf("cleaning up...\n");
  munmap(s_obj.memptr, ByteSize); /* unmap the storage */
  close(s_obj.fd);
  sem_unlink(SemaphoreName);
  sem_close(s_obj.semptr);
  shm_unlink(BackingFile); /* unlink from the backing file */
}


static void sigcb(int signo)
{
  switch (signo) {
  case SIGHUP:
    printf("Get a signal -- SIGHUP\n");
    break;
  case SIGINT:
    printf("Get a signal -- SIGINT\n");
    break;
  case SIGQUIT:
    printf("Get a signal -- SIGQUIT\n");
    break;
  case SIGTERM:
    printf("Get a signal -- SIGTERM\n");
    break;
  }

  clean_up();
  exit(0);
}