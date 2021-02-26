#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "mcm_clients/mcm_client.h"

static void mcm_ind_cb(
    uint32 hndl,
    void *ind_c_struct,
    uint32 ind_len
) {
    int i;
    char *buf = ind_c_struct;
    
    printf("client: enter mcm_ind_cb\n");

    for(i=0;i<ind_len;i++) {
        printf("got:%d\n", buf[i]);
    }
}

int main(void)
{
    uint32 handle;
    mcm_client_init(&handle, mcm_ind_cb);

    while(1) {
        sleep(10);
    }

    return 0;
}