#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>

/*You can use evtest tool to check which event you want to use.*/
const char default_path[] = "/dev/input/event3";

void main(void)
{
    int fd;
    struct input_event event;
    char *path;

    path = (char *)default_path;

    fd = open(path, O_RDONLY);
    if(fd < 0) {
        printf("Fail to open device:%s.\n"
                "Please confirm the path or you have permission to do this.\n", path);
        exit(1);
    }

    printf("Test device: %s.\nWaiting for input...\n", path);

    while(1) {
        if(read(fd, &event, sizeof(event)) == sizeof(event)) {
            if(event.type != EV_SYN)
                printf("Event: time %ld.%ld, type %d, code %d,value %d\n",
                        event.time.tv_sec,event.time.tv_usec,
                        event.type,
                        event.code,
                        event.value);
        }
    }
    close(fd);

    exit(0);
}