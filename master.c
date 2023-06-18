#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "shared.h"
#include <ctype.h>
#include <sys/msg.h>


#define MAX_MESSAGE_SIZE 10000
#define FIFO_NAME4 "myfifo4"
#define MAX_LENGTH 10000 // Maximum length of a column
int shouldContinue = 0, gen = 0, written = 0;


void signal_handler() {
    shouldContinue = 1;
}

void signal_handler2() {
    shouldContinue = 0;
}

char *decodeText(char *text, int column) {
    int shift = column;  // Initial shift value
    int cnt = 0, i = 0;
    char temp[MAX_LENGTH] = {0};

    while (text[i] != '\0') {
        if (text[i] == '(') {
            while (text[i] != ')')
                i++;
        } else if (isspace(text[i]) || text[i] == '.') {
            shift = column;
            temp[cnt] = text[i];
            cnt++;
        } else if (isdigit(text[i])) {
            if (text[i + 1] == '_') {
                if (text[i] == '1')
                    temp[cnt] = '!';
                else if (text[i] == '2')
                    temp[cnt] = '?';
                else if (text[i] == '3')
                    temp[cnt] = ',';
                else if (text[i] == '4')
                    temp[cnt] = ';';
                else if (text[i] == '5')
                    temp[cnt] = ':';
                else if (text[i] == '6')
                    temp[cnt] = '%';
                else if (text[i] == '7')
                    temp[cnt] = '@';
                else if (text[i] == '8')
                    temp[cnt] = '$';
                else if (text[i] == '9')
                    temp[cnt] = '#';
                shift += column;
                i++;
            } else {
                char num[8] = {0};
                int inCnt = 0;
                while (text[i] != '^') {
                    num[inCnt] = text[i];
                    inCnt++;
                    i++;
                }
                inCnt = 1000000 - atoi(num);
                char t[1];
                t[0] = inCnt + '0';
                temp[cnt] = t[0];

            }
            cnt++;
        } else if (isalpha(text[i])) {
            if (text[i] >= 'A' && text[i] <= 'Z') {
                temp[cnt] = 'A' + (text[i] - 'A' - (shift % 26) + 26) % 26;
            } else if (text[i] >= 'a' && text[i] <= 'z') {
                temp[cnt] = 'a' + (text[i] - 'a' - (shift % 26) + 26) % 26;
            }
            cnt++;
            shift += column;  // Increment the shift value for the next character
        }
        i++;
    }

//    printf("%s\n", temp);
    strcpy(text, temp);
    return text;
}

void splitString(char *string, char words[100][100], int *wordCount) {
    char *token = strtok(string, " ");
    *wordCount = 0;

    while (token != NULL) {
        strcpy(words[*wordCount], token);
        (*wordCount)++;
        token = strtok(NULL, " ");
    }
}

void writeMaster(char data[][MAX_LENGTH], int numChild) {
    FILE *file = fopen("master.txt", "w");
    if (file == NULL) {
        printf("Error opening the file.");
        return;
    }

    char words[numChild][100][100];
    int wordCount[numChild];
    int maxWordCount = 0;
    int i, j;

    for (i = 0; i < numChild; i++) {
        splitString(data[i], words[i], &wordCount[i]);
        if (wordCount[i] > maxWordCount)
            maxWordCount = wordCount[i];
    }

    for (j = 0; j < maxWordCount; j++) {
        for (i = 0; i < numChild; i++) {
            if (j < wordCount[i] && strcmp(words[i][j], "alright") != 0) {
                fprintf(file, "%s ", words[i][j]);
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
}


int main() {
    // ftok to generate unique key
    key_t keyq = 6565;
    // msgget creates a message queue and returns identifier
    int msgid = msgget(keyq, 0666 | IPC_CREAT);
    if (msgid == -1) {                                                          //msgqueue
        perror("msgget");
        exit(1);
    }
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    struct sigaction sa1 = {0};
    sa1.sa_handler = &signal_handler2;
    sigaction(SIGUSR2, &sa1, NULL); // set up signal handler for SIGUSR1
    char columns[3];
    int fd = open(FIFO_NAME4, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1)
        perror("Error reading FIFO file\n");
    //printf("Number of columns 1 master %s:\n", columns);
    int numEncode = atoi(columns);
    close(fd);
    char unique_values[numEncode][SHARED_MEMORY_SIZE];
    while (1) {
        if (shouldContinue == 1) {
            key_t key1 = 123432;
            int shmid1 = shmget(key1, sizeof(struct shared_data), 0);
            if (shmid1 == -1) {
                perror("shmget");
                exit(1);                                                                                    //shared memory attach
            }

            struct shared_data *shared_memory1 = (struct shared_data *) shmat(shmid1, NULL,
                                                                              0);                                     //openGL
            if (shared_memory1 == (struct shared_data *) (-1)) {
                perror("shmat");
                exit(1);
            }

// Receive and print messages from spies
            char value[MAX_MESSAGE_SIZE];

            if (msgrcv(msgid, value, sizeof(value), 0, 0) == -1) {                  //receivng msgs from spies
                perror("msgrcv");
            } else {
                printf("Master received message: %s\n", value);
            }



//            fflush(stdout);
            int isValuePresent = 0;
            char num[3];
            int column, cnt = 0, cnt2 = 0;

            while (value[cnt] != ')') {
                if (isdigit(value[cnt])) {
                    num[cnt2] = value[cnt];
                    cnt2++;
                }
                cnt++;
            }
            num[cnt2] = '\0';
            column = atoi(num);

            char temp[100] = {0};
            strcpy(temp, value);
            for (int i = 0; i < numEncode; i++) {
                if (strcmp(unique_values[i], temp) == 0) {
                    isValuePresent = 1;
                    break;
                }
            }

            if (!isValuePresent && !written) {
                strcpy(unique_values[column - 1], temp);
                //strcpy(shared_memory1->value[column - 1], unique_values[column - 1]);
                for (int i = 0; i < numEncode; i++) {
                    strcpy(shared_memory1->value[i],
                           unique_values[i]);                                             //saving them into array if they were new untill we are done
                    //printf("Message written in index %d --> %s\n", i, shared_memory1->value[i]);
                }
                gen++;
                //printf("%d\n", gen);
            }

            if (gen == numEncode) {
                if (!written) {
                    for (int i = 0; i < gen; i++) {
//                        printf("Encoded message written in index %d --> %s\n", i, unique_values[i]);
                        decodeText(unique_values[i], i + 1);
                        printf("Decoded message written in index %d --> %s\n\n", i, unique_values[i]);
                    }
                    written = 1;
                    sleep(1);
                    kill(getppid(), SIGUSR2);
                    kill(getpid(), SIGUSR2);
                    sleep(1);
                    writeMaster(unique_values,
                                numEncode);                              //writting to file and resetting everything for next round
                    memset(unique_values, '\0', sizeof(unique_values));
                    for (int i = 0; i < numEncode; i++) {
                        memset(shared_memory1->value[i], '\0', SHARED_MEMORY_SIZE);
                    }
                    gen = 0, written = 0;
                }
            }
            shmdt(shared_memory1);
        } else {
            key_t key1 = 123432;
            int shmid1 = shmget(key1, sizeof(struct shared_data), 0);
            if (shmid1 == -1) {
                perror("shmget");                                                                               //just reset everything
                exit(1);
            }

            struct shared_data *shared_memory1 = (struct shared_data *) shmat(shmid1, NULL, 0);
            if (shared_memory1 == (struct shared_data *) (-1)) {
                perror("shmat");
                exit(1);
            }
            gen = 0, written = 0;
            memset(unique_values, '\0', sizeof(unique_values));
            for (int i = 0; i < numEncode; i++) {
                memset(shared_memory1->value[i], '\0', SHARED_MEMORY_SIZE);
            }
            sleep(1);
            if (shmdt(shared_memory1) == -1) {
                perror("shmdt");
            }
        }
    }
}