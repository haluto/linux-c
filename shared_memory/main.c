#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>


#define DEFAULT_TOKEN "123456789abcdef0"
#define TOKEN_SIZE 16
#define SHM_KEY_FTOK_PATHNAME   "/home/android/"
#define SHM_KEY_FTOK_PROJ_ID    100
enum SessionTokenId {
    A_TOKEN_ID    = 0,
    B_TOKEN_ID    = 1,
    SESSION_TOKEN_COUNT
};

static int get_shm_session_token(char* out, int id);
static int gen_shm_session_token(int id);
static int del_shm_session_token();

int main(void)
{
    char session_token[TOKEN_SIZE+1] = {'\0'};
    //gen_shm_session_token(A_TOKEN_ID);
    //get_shm_session_token(session_token, A_TOKEN_ID);
    //printf("got token: %s\n", session_token);
    del_shm_session_token();
}

static int del_shm_session_token() {

    int shm_token_id = -1;
    int shm_size = TOKEN_SIZE*SESSION_TOKEN_COUNT;
    key_t shm_token_key;

    if((shm_token_key = ftok(SHM_KEY_FTOK_PATHNAME, SHM_KEY_FTOK_PROJ_ID)) == -1) {
        printf("gen key failed\n");
        return -1;
    }
    shm_token_id = shmget(shm_token_key, shm_size, 0644 | IPC_CREAT);
    // delete shared memory
    if(shm_token_id > 0) {
        if (shmctl(shm_token_id, IPC_RMID, 0) == -1) {
            printf("shmctl delete shared memory failed\n");
        }
        shm_token_id = -1;
        return -1;
    }

    return 0;
}

static int get_shm_session_token(char* out, int id) {

    int shm_token_id = -1;
    void *shm_token_addr = NULL;
    int shm_size = TOKEN_SIZE*SESSION_TOKEN_COUNT;
    key_t shm_token_key;

    if((shm_token_key = ftok(SHM_KEY_FTOK_PATHNAME, SHM_KEY_FTOK_PROJ_ID)) == -1) {
        printf("gen key failed\n");
        return -1;
    }

    // 1. create shared memory
    shm_token_id = shmget(shm_token_key, shm_size, 0644 | IPC_CREAT);
    printf("shm_token_id= %d\n", shm_token_id);
    if (shm_token_id == -1) {
        printf("shmget failed\n");
        goto ERROR;
    }

    // 2. attach shared memory
    shm_token_addr = shmat(shm_token_id, NULL, 0);
    if (shm_token_addr == (void *)-1) {
        printf("shmat failed\n");
        goto ERROR;
    }

    // 3. read data to shared memory
    memset(out, '\0', TOKEN_SIZE);
    memcpy(out, shm_token_addr+(id*TOKEN_SIZE), TOKEN_SIZE);

    // 4. detach shared memory
    if (shmdt(shm_token_addr) == -1) {
        printf("shmdt failed\n");
        return -1;
    }

    return 0;

ERROR:
    memset(out, '\0', TOKEN_SIZE);
    memcpy(out, DEFAULT_TOKEN, strlen(DEFAULT_TOKEN)+1);

    return -1;
}


static void gen_random(char *s, const int len)
{
    const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

static int gen_shm_session_token(int id)
{
    int shm_token_id = -1;
    void *shm_token_addr = NULL;
    int shm_size = TOKEN_SIZE*SESSION_TOKEN_COUNT;
    char data[TOKEN_SIZE+1] = {'\0'};
    key_t shm_token_key;

    if((shm_token_key = ftok(SHM_KEY_FTOK_PATHNAME, SHM_KEY_FTOK_PROJ_ID)) == -1) {
        printf("gen key failed\n");
        return -1;
    }

    // 1. create shared memory
    if(shm_token_id < 0) {
        shm_token_id = shmget(shm_token_key, shm_size, 0644 | IPC_CREAT);
        if (shm_token_id == -1) {
            printf("shmget failed\n");
            return -1;
        }
    }

    // 2. attach shared memory
    shm_token_addr = shmat(shm_token_id, NULL, 0);
    if (shm_token_addr == (void *)-1) {
        printf("shmat failed\n");
        return -1;
    }

    // 3. generate random session token
    gen_random(data, TOKEN_SIZE);
    data[TOKEN_SIZE] = '\0';
    printf("data size: %d, session: %s\n", TOKEN_SIZE, data);

    // 4. write data to shared memory
    memset(shm_token_addr+(id*TOKEN_SIZE), '\0', TOKEN_SIZE);
    memcpy(shm_token_addr+(id*TOKEN_SIZE), &data, TOKEN_SIZE);

    // 4. detach shared memory
    if (shmdt(shm_token_addr) == -1) {
        printf("shmdt failed\n");
        return -1;
    }
    printf("shmat write done !\n");
    return 0;
}