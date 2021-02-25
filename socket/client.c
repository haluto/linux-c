#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define MY_IP    "127.0.0.1"
#define MY_PORT  5678

#define MAXLINE  4096

int main(void)
{
    int sockfd;
    char recvline[MAXLINE], sendline[MAXLINE];
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MY_PORT);
    if(inet_pton(AF_INET, MY_IP, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", MY_IP);
        exit(0);
    }

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    while(1) {
        printf("send msg to server: \n");
        fgets(sendline, MAXLINE, stdin);
        if(strncmp(sendline, "quit", strlen("quit")) == 0) {
            printf("sockfd= %d, quit...\n", sockfd);
            break;
        }
        if(send(sockfd, sendline, strlen(sendline), 0) < 0) {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
    }

    close(sockfd);
    exit(0);
}