#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include "mcm_client.h"

/*********************
 *      DEFINES
 *********************/
#define MAX_CLIENTS 10

/**********************
 *  STATIC VARIABLES
 **********************/
static mcm_client_table_t s_client_table[MAX_CLIENTS];
static pthread_mutex_t client_table_mutex;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int Init(uint32 *handle, char *path);
static int Deinit(uint32 *handle, char *path);
static void *client_thread_cb(void *arg);

static void mcm_client_srv_table_add(uint32 *mcm_client_handle, mcm_client_ind_cb ind_cb);

/**
 * MCM Client.
*/
/*APIs for clients.*/
uint32 mcm_client_init(uint32 *mcm_client_handle, mcm_client_ind_cb ind_cb)
{
    int clientSock = -1;
    char path[] = "/tmp/client_sun_path.0";
    Init(&clientSock, path);
    printf("after Init: handle = %d\n", clientSock);
    if(clientSock > 0) {
        *mcm_client_handle = clientSock;
    } else {
        *mcm_client_handle = 0;
    }
    if(*mcm_client_handle > 0 && ind_cb != NULL) {
        pthread_t pid = NULL;
        static mcm_client_table_t client_table;
        client_table.mcm_handle = *mcm_client_handle;
        client_table.ind_cb = ind_cb;
        printf("mcm_client_init: handle = %d\n", client_table.mcm_handle);
        pthread_create(&pid, NULL, &client_thread_cb, &client_table);
    }

    //TODO: return value.
    return 0;
}

static void *client_thread_cb(void *arg)
{
    mcm_client_table_t *p_table = arg;
    ssize_t  recvedSize = 0;
    unsigned char buf[4];
    struct sockaddr_un serverSockaddr;
    socklen_t serverSockaddrLen;
    int ret = 0;
    int i = 0;
    
    
    printf("client_thread_cb: handle = %d\n", p_table->mcm_handle);
    while(!ret) {
        memset(buf, 0x00, sizeof(buf));
        serverSockaddrLen = sizeof(struct sockaddr_un);
        printf("recvfrom\n");
        recvedSize = recvfrom(p_table->mcm_handle, buf, sizeof(buf), 0,
            (struct sockaddr *)&serverSockaddr, &serverSockaddrLen);
        if(-1 == recvedSize) {
            printf("%s: ERROR! recvfrom: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }

        p_table->ind_cb(p_table->mcm_handle, buf, sizeof(buf));
    }
}

static int Init(uint32 *handle, char *path)
{
    int ret = 0;
    static char path1[100];
    memset(path1, 0, 100);
    strcpy(path1, path);
    printf("path1:%s\n",path1);
    // TODO: why path1 as param will fail....
    const struct sockaddr_un  clientSockaddr = {AF_UNIX, "/tmp/client_sun_path.0"/*path1*/};

    *handle = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(-1 == *handle) {
        printf("%s: ERROR! socket: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }
    if(!ret && unlink(path) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }

    //reuse the connect.
    //int opt = 1;
    //setsockopt(*handle,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof( opt ));

    if(!ret && bind(*handle, (struct sockaddr *)&clientSockaddr, sizeof(struct sockaddr_un))) {
        printf("%s: ERROR! bind: %s\n", __FUNCTION__, strerror(errno));
        ret = 0;//ret = -1; //if bind returns Address already in use, just reuse it. dont Deinit
    }

    if(0 != ret) {
        Deinit(handle, path);
    }

    return ret;
}

static int Deinit(uint32 *handle, char *path)
{
    int ret = 0;

    if(-1 != *handle) {
        if (-1 == close(*handle)) {
            printf("%s: ERROR! close: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }
    }
    *handle = -1;
    if(unlink(path) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }

    printf("Deinit\n");
    return ret;
}


/**
 * MCM Client Service.
*/
/*API which need server to init.*/
void mcm_client_srv_table_init(void)
{
    memset(s_client_table, 0, sizeof(s_client_table));
    pthread_mutex_init(&client_table_mutex, NULL);
}
static void mcm_client_srv_table_add(uint32 *mcm_client_handle, mcm_client_ind_cb ind_cb)
{

}