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
#include <sys/msg.h>

#define MAX_MESSAGE_SIZE 10000
#define FIFO_NAME3 "myfifo3"
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

int main(int argc, char *argv[]) {
    int spyNum = atoi(argv[1]);
    printf("I'm spy %d ......\n", spyNum);
    key_t keyq = 6565;
    // Generate a unique key
    // Create a message queue
    int msgid = msgget(keyq, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }    //initializing message queue...
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    struct sigaction sa1 = {0};
    sa1.sa_handler = &signal_handler2;
    sigaction(SIGUSR2, &sa1, NULL); // set up signal handler for SIGUSR1
    int fd;
    char columns[3];
    sleep(2);
    fd = open(FIFO_NAME3, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1)
        perror("Error reading FIFO file\n");
    printf("Number of columns2 spy %s:\n", columns);
    int numEncode = atoi(columns);
    close(fd);

    while (1) {
        if (shouldContinue == 1) {
            key_t key, key5;
            int shmid, shmid5;
            struct shared_data *shared_memory;
            struct shared_data3 *shared_memory5;

            // Generate the same key as in Process 1
            key = 141414;


            // Get the existing shared memory segment
            shmid = shmget(key, sizeof(struct shared_data), 0);
            if (shmid == -1) {
                perror("shmget");
                exit(1);
            }

            // Attach the shared memory segment
            shared_memory = (struct shared_data *) shmat(shmid, NULL, 0);                   //acessing shared memory
            if (shared_memory == (struct shared_data *) -1) {
                perror("shmat");
                exit(1);
            }

            // Create the second shared memory segment
            key5 = 1234321321;
            shmid5 = shmget(key5, sizeof(struct shared_data3), IPC_CREAT | 0666);
            if (shmid5 == -1) {
                perror("shmget");
                exit(1);
            }

            // Attach the second shared memory segment
            shared_memory5 = (struct shared_data3 *) shmat(shmid5, NULL, 0);                    //openGL
            if (shared_memory5 == (struct shared_data3 *) -1) {
                perror("shmat");
                exit(1);
            }
            //read from shared memo...
            srandom(time(NULL) + getpid());
            int random_number1 = random() % numEncode;
            sleep(1);
            char *value = shared_memory->value[random_number1];                                                     //acess random
            char mtext[MAX_MESSAGE_SIZE];
            strncpy(mtext, value, MAX_MESSAGE_SIZE);
            strcpy(shared_memory5->spies[spyNum], value);
            printf("I'm spy %d i sent the msg: %s\n", spyNum, value);

// Send the message through the message queue
            if (msgsnd(msgid, &mtext, strlen(mtext) + 1, 0) ==
                -1) {                        //sending messages to message queue for master spy
                perror("msgsnd");
                exit(1);
            }

            printf("%s sending..\n", mtext);

            // Close the message queue
            sleep(1);
            if (shmdt(shared_memory) == -1) {
                perror("shmdt");
                exit(1);
            }
            if (shmdt(shared_memory5) == -1) {
                perror("shmdt");
                exit(1);
            }

        } else {
            sleep(1);
        }
    }
}