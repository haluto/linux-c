#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>  //need ignore socket signal

/*********************
 *      DEFINES
 *********************/
#define MY_IP    "127.0.0.1"
#define MY_PORT  5678
#define BACKLOG  100

#define MAXLINE  4096

#define MAX_CLIENTS 5

/**********************
 *  STATIC VARIABLES
 **********************/
static int s_client_table[MAX_CLIENTS];
static pthread_mutex_t client_table_mutex;
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void *client_cb(void *arg);
static void client_table_init(void);
static void client_table_add(int fd);
static void client_table_del(int fd);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int main(void)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t len = 0;
    int sock_fd = -1;
    int client_fd = -1;

    int ret = -1;
    pthread_t tid = -1;

    client_table_init();

    signal(SIGPIPE, SIG_IGN); //ignore pipe broken.
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock_fd) {
        perror("socket");
        return -1;
    }
    printf("sock_fd= %d\n", sock_fd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MY_PORT);
    server_addr.sin_addr.s_addr = inet_addr(MY_IP);
    ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(-1 == ret) {
        perror("bind");
        return -1;
    }
    printf("bind ok\n");

    ret = listen(sock_fd, BACKLOG);
    if(-1 == ret) {
        perror("listen");
        return -1;
    }
    printf("listen ok\n");

    while(1) {
        client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);
        if(-1 == client_fd) {
            perror("listen");
            return -1;
        }
        ret = pthread_create(&tid, NULL, &client_cb, &client_fd);
        printf("client-%d joined\n", client_fd);
        if(ret != 0) {
            printf("pthread_create error\n");
            return -1;
        }
        client_table_add(client_fd);

    }
    close(sock_fd);
    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void *client_cb(void *arg)
{
    char recv_buf[MAXLINE];
    char send_buf[MAXLINE]; // should + several bites.
    int *p_client_fd = arg;
    int client_fd = *p_client_fd;
    int ret = 1;
    int i;
    while(1) {
        memset(recv_buf,0,sizeof(recv_buf));
        ret = recv(client_fd, recv_buf, sizeof(recv_buf), 0);
        if(0 == ret) {
            break;//return 0 means the pipe is broken.
        }
        printf("client-%d: %s\n", client_fd, recv_buf);
        memset(send_buf,0,sizeof(send_buf));
        sprintf(send_buf, "client-%d: %s", client_fd, recv_buf);
        for(i=0;i<MAX_CLIENTS;i++) {
            if(s_client_table[i] != 0) {
                send(s_client_table[i], &send_buf, sizeof(send_buf), 0);
            }
        }
    }
    printf("client-%d over\n", client_fd);
    close(client_fd);
    client_table_del(client_fd);
    memset(send_buf,0,sizeof(send_buf));
    sprintf(send_buf, "<client-%d left the room>", client_fd);
    for(i=0;i<MAX_CLIENTS;i++) {
        if(s_client_table[i] != 0) {
            send(s_client_table[i], &send_buf, sizeof(send_buf), 0);
        }
    }
}


/*poor, just for demo.*/
static void client_table_init(void)
{
    memset(s_client_table, 0, sizeof(s_client_table));
    pthread_mutex_init(&client_table_mutex, NULL);
}
static void client_table_add(int fd)
{
    int i;
    pthread_mutex_lock(&client_table_mutex);
    for(i=0;i<MAX_CLIENTS;i++) {
        if(s_client_table[i] == 0) {
            s_client_table[i] = fd;
            break;
        }
    }
    pthread_mutex_unlock(&client_table_mutex);
}
static void client_table_del(int fd)
{
    int i;
    pthread_mutex_lock(&client_table_mutex);
    for(i=0;i<MAX_CLIENTS;i++) {
        if(s_client_table[i] == fd) {
            s_client_table[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&client_table_mutex);
}