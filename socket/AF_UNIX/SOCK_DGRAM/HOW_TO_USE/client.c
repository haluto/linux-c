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

static int  clientSock = -1;

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
    const struct sockaddr_un  clientSockaddr = {AF_UNIX, CLIENT_SUN_PATH};
    int optval = 1;

    clientSock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(-1 == clientSock) {
        printf("%s: ERROR! socket: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }
    if(!ret && unlink(CLIENT_SUN_PATH) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }
    //if (!ret && -1 == setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) {
    //   printf("%s: ERROR! setsockopt: %s\n", __FUNCTION__, strerror(errno));
    //   ret = -1;
    //}
    if(!ret && bind(clientSock, (struct sockaddr *)&clientSockaddr, sizeof(struct sockaddr_un))) {
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

    if(-1 != clientSock) {
        if (-1 == close(clientSock)) {
            printf("%s: ERROR! close: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }
    }
    clientSock = -1;
    if(unlink(CLIENT_SUN_PATH) && ENOENT != errno) {
        printf("%s: ERROR! unlink: %s\n", __FUNCTION__, strerror(errno));
        ret = -1;
    }

    return ret;
}

static int MsgLoop()
{
    ssize_t  recvedSize = 0;
    unsigned char buf[4];
    struct sockaddr_un serverSockaddr;
    socklen_t serverSockaddrLen;
    int ret = 0;
    int i = 0;

    while(!ret) {
        memset(buf, 0x00, sizeof(buf));
        serverSockaddrLen = sizeof(struct sockaddr_un);
        recvedSize = recvfrom(clientSock, buf, sizeof(buf), 0,
            (struct sockaddr *)&serverSockaddr, &serverSockaddrLen);
        if(-1 == recvedSize) {
            printf("%s: ERROR! recvfrom: %s\n", __FUNCTION__, strerror(errno));
            ret = -1;
        }

        printf("received %d bytes: ", recvedSize);
        for(i = 0; i < recvedSize; i++) {
            printf("%02x,", buf[i]);
        }
        printf("\n");
    }

    return ret;
}