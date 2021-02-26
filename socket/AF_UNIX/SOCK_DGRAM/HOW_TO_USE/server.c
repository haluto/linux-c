#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SERVER_SUN_PATH "/tmp/server_sun_path.0"
#define CLIENT_SUN_PATH "/tmp/client_sun_path.0"

static int Init();
static int Deinit();
static int MsgLoop();

static int  serverSock = -1;

int main(int argc, char *argv[])
{
    int ret = 0;

    ret = Init();
    if(!ret) {
        ret = MsgLoop();
    }
    Deinit();

    printf("%s: ret=%d\n", __FUNCTION__, ret);

    return ret;
}

static int Init()
{
    int ret = 0;
    const struct sockaddr_un  serverSockaddr = {AF_UNIX, SERVER_SUN_PATH};
    int optval = 1;

    serverSock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(-1 == serverSock) {
        printf("%s: ERROR! socket: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }
    if(!ret && unlink(SERVER_SUN_PATH) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }
    //if (!ret && -1 == setsockopt(serverSock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int))) {
    //   printf("%s: ERROR! setsockopt: %s\n", __FUNCTION__, strerror(errno));
    //   ret = -1;
    //}
    if(!ret && bind(serverSock, (struct sockaddr *)&serverSockaddr, sizeof(struct sockaddr_un))) {
        printf("%s: ERROR! bind: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }

    if(0 != ret) {
        Deinit();
    }

    return ret;
}

static int Deinit()
{
    int ret = 0;

    if(-1 != serverSock) {
        if(-1 == close(serverSock)) {
            printf("%s: ERROR! close: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }
    }
    serverSock = -1;
    if(unlink(SERVER_SUN_PATH) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }

    return ret;
}

static int MsgLoop()
{
    ssize_t  sentSize = 0;
    unsigned char buf[4] = {0x4, 0x3, 0x2, 0x1};
    const struct sockaddr_un  clientSockaddr = {AF_UNIX, CLIENT_SUN_PATH};
    int ret = 0;

    while(!ret) {
        sentSize = sendto(serverSock, buf, sizeof(buf), 0,
            (struct sockaddr *)&clientSockaddr, sizeof(struct sockaddr_un));
        if(-1 == sentSize && ECONNREFUSED != errno && ENOENT != errno) {
            printf("%s: ERROR! sendto: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }

        printf("sent %d bytes\n", sentSize);
        sleep(1);
    }

    return ret;
}