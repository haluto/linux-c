#ifndef __SEM_H__
#define __SEM_H__

#define WITH_SEM 1

#define SHM_KEY 0x12345
#define SEM_KEY 0x54321
#define MAX_TRIES 20

struct shmseg {
    int cntr;
    int write_complete;
    int read_complete;
};


#endif /*__SEM_H__*/
