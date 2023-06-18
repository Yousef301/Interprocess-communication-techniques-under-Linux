#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shared.h"
#include <fcntl.h>
#include <signal.h>

#define MAX_LENGTH 10000 // Maximum length of a column

sem_t *semaphoreEncode, *semaphoreEncodeWait;
char column[MAX_LENGTH];
int numColumn;
//pid_t grandParentPID;

void signal_handler() {
    key_t key = 141414;

    int shmid = shmget(key, sizeof(struct shared_data), 0);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL, 0);
    if (shared_memory == (struct shared_data *) (-1)) {
        perror("shmat");
        exit(1);
    }
    sem_wait(semaphoreEncode);
    printf("I'm child %d: %s\n", numColumn - 1, column);
    strcpy(shared_memory->value[numColumn - 1],
           column);                                            // its time to work guys, encode and put on the shared memory
    sem_post(semaphoreEncode);
    sem_post(semaphoreEncodeWait);
    usleep(numColumn * 10000);
    shmdt(shared_memory);
}

int handleSpecialEncode(int cnt, char *temp, int special) {
    char temp1 = (char) (special);
    char temp2[3];
    sprintf(temp2, "%d_", temp1);
    strcat(temp, temp2);
    return cnt = strlen(temp) - 1;
}

void encodeText(char *text, int column) {
    int shift = column;  // Initial shift value
    int cnt = 0;
    char temp[MAX_LENGTH] = {0};

    for (int i = 0; text[i] != '\0'; i++) {
        if (isspace(text[i]) || text[i] == '.') {
            shift = column;
            temp[cnt] = text[i];
        } else if (isdigit(text[i])) {
            int temp1 = 1000000 - (text[i] - '0');
            char temp3[15] = {0};
            sprintf(temp3, "%d^", temp1);
            strcat(temp, temp3);
            cnt = strlen(temp) - 1;
            shift += column;
        } else if (isalpha(text[i])) {
            char base = isupper(text[i]) ? 'A' : 'a';  // Determine the base value ('A' or 'a')
            temp[cnt] = ((text[i] - base + shift) % 26) + base;  // Apply the shift and wrap around

            shift += column;  // Increment the shift value for the next character
        } else {
            if (text[i] == '!')
                cnt = handleSpecialEncode(cnt, temp, 1);
            else if (text[i] == '?')
                cnt = handleSpecialEncode(cnt, temp, 2);
            else if (text[i] == ',')
                cnt = handleSpecialEncode(cnt, temp, 3);
            else if (text[i] == ';')
                cnt = handleSpecialEncode(cnt, temp, 4);
            else if (text[i] == ':')
                cnt = handleSpecialEncode(cnt, temp, 5);
            else if (text[i] == '%')
                cnt = handleSpecialEncode(cnt, temp, 6);
            else if (text[i] == '@')
                cnt = handleSpecialEncode(cnt, temp, 7);
            else if (text[i] == '$')
                cnt = handleSpecialEncode(cnt, temp, 8);
            else if (text[i] == '#')
                cnt = handleSpecialEncode(cnt, temp, 9);
            shift += column;
        }
        cnt++;
    }
    strcpy(text, temp);
    char prefix[5] = {0};
    sprintf(prefix, "(%d)", column);
    size_t prefixLength = strlen(prefix);
    memmove(text + prefixLength, text, strlen(text) + 1);
    memcpy(text, prefix, prefixLength);
}


int main(int argc, char *argv[]) {
    semaphoreEncode = sem_open("/write_semaphore", O_CREAT, 0666, 0);
    if (semaphoreEncode == SEM_FAILED) {
        perror("sem_open");                                                                     //semaphore for sync between the encoders
        exit(1);
    }
    semaphoreEncodeWait = sem_open("/wait_encode_semaphore", O_CREAT, 0666,
                                   0);       // semaphore to sync other processes that we finished encoding
    if (semaphoreEncode == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    strcpy(column, argv[2]);
    numColumn = atoi(argv[1]);
    //grandParentPID = atoi(argv[3]);
    encodeText(column, numColumn);                                          //encoding the text
    while (1) {
        sleep(1);                                                                   //in normal cases just dont do anything please
    }
}