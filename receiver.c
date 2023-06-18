#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "shared.h"
#include <time.h>
#include <ctype.h>

#define FIFO_NAME2 "myfifo2"
#define MAX_LENGTH 10000 // Maximum length of a column

int shouldContinue = 0, gen = 0, written = 0;

void signal_handler() {
    shouldContinue = 1;
    printf("SIGNAL 1 Rec\n");
}

void signal_handler2() {
    shouldContinue = 0;
    printf("SIGNAL 2 Rec\n");
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

void writeReceiver(char data[][MAX_LENGTH], int numChild) {
    FILE *file = fopen("receiver.txt", "w");
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
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    struct sigaction sa1 = {0};
    sa1.sa_handler = &signal_handler2;
    sigaction(SIGUSR2, &sa1, NULL); // set up signal handler for SIGUSR1
    int fd;
    char columns[3];
    sleep(2);
    fd = open(FIFO_NAME2, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1)
        perror("Error reading FIFO file\n");
    printf("Number of columns2 rec %s:\n", columns);
    int numEncode = atoi(columns);
    char unique_values[numEncode][SHARED_MEMORY_SIZE];
    close(fd);
    while (1) {
        if (shouldContinue == 1) {
            key_t key = 141414;
            int shmid = shmget(key, sizeof(struct shared_data), 0);  // Get the existing shared memory segment
            struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL,
                                                                             0);  // Attach shared memory segment
            key_t key2 = 1234321;
            int shmid2 = shmget(key2, sizeof(struct shared_data),
                                IPC_CREAT | 0666);  // Create second shared memory segment
            struct shared_data *shared_memory2 = (struct shared_data *) shmat(shmid2, NULL,
                                                                              0);  // Attach second shared memory segment
            srandom(time(NULL) + getpid());
            int random_number1 = random() %
                                 (numEncode);                            //same as master spy instead he acess the shared memory
            sleep(1);
            // Access shared memory, read value, and add it to the array if not already present
//            char *value = "";
//            if (gen < numEncode)
            char *value = shared_memory->value[random_number1];
            int isValuePresent = 0;
//            gen++;
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
//            printf("Before decoding: %s\n", value);
            char temp[100] = {0};
            strcpy(temp, value);
//            decodeText(temp, column);
//            printf("After decoding: %s\n", temp);
//            sleep(1);
            for (int i = 0; i < numEncode; i++) {

                if (strcmp(unique_values[i], temp) == 0) {
                    isValuePresent = 1;
                    break;
                }
            }

            if (!isValuePresent && !written) {
                strcpy(unique_values[column - 1], temp);
                for (int i = 0; i < numEncode; i++) {
                    strcpy(shared_memory2->value[i], unique_values[i]);
                    //printf("Message written in index %d --> %s\n", i, shared_memory1->value[i]);
                }
//                printf("Message written in index %d --> %s\n", column - 1, unique_values[column - 1]);
                gen++;
                printf("%d\n", gen);
                fflush(stdout);
            }

            if (gen == numEncode) {
                if (!written) {
                    for (int i = 0; i < gen; i++) {
//                        printf("Encoded message written in index %d --> %s\n", i, unique_values[i]);
                        decodeText(unique_values[i], i + 1);
                        //printf("Decoded message written in index %d --> %s\n\n", i, unique_values[i]);
                    }
                    sleep(1);
                    written = 1;
                    kill(getppid(), SIGUSR1);
                    kill(getpid(), SIGUSR2);
                    writeReceiver(unique_values, numEncode);
                    memset(unique_values, '\0', sizeof(unique_values));
                    sleep(1);
                    for (int i = 0; i < numEncode; i++) {
                        memset(shared_memory2->value[i], '\0', SHARED_MEMORY_SIZE);
                    }
                    written = 0;
                    gen = 0;
                }
            }
            shmdt(shared_memory2);
            shmdt(shared_memory);
        } else {
            written = 0;
            gen = 0;
            char *shared_memory_key = "shared_memory_key";
            size_t shared_memory_size = sizeof(struct shared_data);

            // Get the key for the existing shared memory segment
            key_t key = 141414;

            // Get the existing shared memory segment
            int shmid = shmget(key, shared_memory_size, 0);
            if (shmid == -1) {
                perror("shmget");
                exit(1);
            }

            // Attach the shared memory segment
            struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL, 0);
            if (shared_memory == (void *) -1) {
                perror("shmat");
                exit(1);
            }

            // Create the second shared memory segment
            key_t key2 = 1234321;
            int shmid2 = shmget(key2, shared_memory_size, IPC_CREAT | 0666);
            if (shmid2 == -1) {
                perror("shmget");
                exit(1);
            }

            // Attach the second shared memory segment
            struct shared_data *shared_memory2 = (struct shared_data *) shmat(shmid2, NULL, 0);
            if (shared_memory2 == (void *) -1) {
                perror("shmat");
                exit(1);
            }
            memset(unique_values, '\0', sizeof(unique_values));
            for (int i = 0; i < numEncode; i++) {
                memset(shared_memory2->value[i], '\0', SHARED_MEMORY_SIZE);
            }
            sleep(1);
            if (shmdt(shared_memory2) == -1) {
                perror("shmdt");
                exit(1);
            }
            if (shmdt(shared_memory) == -1) {
                perror("shmdt");
                exit(1);
            }
        }
    }
}