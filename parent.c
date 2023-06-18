#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "shared.h"
#include <errno.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/msg.h>

#define FIFO_NAME "myfifo"
#define FIFO_NAME1 "myfifo1"
#define FIFO_NAME2 "myfifo2"
#define FIFO_NAME3 "myfifo3"
#define FIFO_NAME4 "myfifo4"
#define FIFO_NAME5 "myfifo5"

#define SEMAPHORE_PREFIX "/my_semaphore"

int roundNum = 1, numEncode = 0, spyWin, receiverWin = 0, spyScore, receiverScore = 0;

sem_t *semaphoreEncode, *semaphoreEncodeWait;
pid_t *child_pids;                    //remmember to put free at last
pid_t disPID;

void signal_handler() {
    receiverWin = 1;
}

//getting who got the messages first
void signal_handler2() {
    spyWin = 1;
}

int main() {
    if (mkfifo(FIFO_NAME, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    if (mkfifo(FIFO_NAME1, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    if (mkfifo(FIFO_NAME2, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    if (mkfifo(FIFO_NAME3, 0777) == -1) {
        if (errno !=
            EEXIST) {                                                  //fifos to read the num encode from the sender and sending them to other procs
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    if (mkfifo(FIFO_NAME4, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    if (mkfifo(FIFO_NAME5, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    semaphoreEncode = sem_open("/write_semaphore", O_CREAT, 0666, 0);
    if (semaphoreEncode == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_init(semaphoreEncode, 1,
             1);                                        //semaphores to sync decodes and wait for them to finish to signal other procs to start working

    semaphoreEncodeWait = sem_open("/wait_encode_semaphore", O_CREAT, 0666, 0);
    if (semaphoreEncode == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_init(semaphoreEncodeWait, 1, 0);
    key_t keys = 141414;

    int shmid = shmget(keys, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL, 0);
    if (shared_memory == (struct shared_data *) (-1)) {
        perror("shmat");
        exit(1);
    }

    key_t key1 = 123432;
    int shmid1 = shmget(key1, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid1 == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shared_memory1 = (struct shared_data *) shmat(shmid1, NULL, 0);
    if (shared_memory1 == (struct shared_data *) (-1)) {
        perror("shmat");
        exit(1);
    }

    key_t key2 = 1234321;
    int shmid2 = shmget(key2, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid2 == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shared_memory2 = (struct shared_data *) shmat(shmid2, NULL,
                                                                      0);                         //creating all shared memories
    if (shared_memory2 == (struct shared_data *) (-1)) {
        perror("shmat");
        exit(1);
    }

    key_t key3 = 12343211;
    int shmid3 = shmget(key3, sizeof(struct shared_data1), IPC_CREAT | 0666);
    if (shmid3 == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data1 *shared_memory3 = (struct shared_data1 *) shmat(shmid3, NULL, 0);
    if (shared_memory3 == (struct shared_data1 *) (-1)) {
        perror("shmat");
        exit(1);
    }

    key_t key4 = 1234321123;
    int shmid4 = shmget(key4, sizeof(struct shared_data2), IPC_CREAT | 0666);
    if (shmid4 == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data2 *shared_memory4 = (struct shared_data2 *) shmat(shmid4, NULL, 0);
    if (shared_memory4 == (struct shared_data2 *) (-1)) {
        perror("shmat");
        exit(1);
    }

    key_t key5 = 1234321321;
    int shmid5 = shmget(key5, sizeof(struct shared_data3), IPC_CREAT | 0666);
    if (shmid5 == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared_data3 *shared_memory5 = (struct shared_data3 *) shmat(shmid5, NULL, 0);
    if (shared_memory5 == (struct shared_data3 *) (-1)) {
        perror("shmat");
        exit(1);
    }

    int helperNum, spiesNum, win, lose;

    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {                                     //reading data for txt file
        printf("Failed to open the file.\n");
        return 1;
    }
    char line[100];

    while (fgets(line, sizeof(line), file)) {
        // Split the line into key and value
        char *key = strtok(line, ":");
        char *value = strtok(NULL, ":");

        // Remove leading/trailing whitespaces from key and value
        key = strtok(key, " \t\n");
        value = strtok(value, " \t\n");

        // Check the key and update the corresponding variable
        if (strcmp(key, "HELPERS") == 0) {
            helperNum = atoi(value);
        } else if (strcmp(key, "SPIES") == 0) {                                         //data
            spiesNum = atoi(value);
        } else if (strcmp(key, "WIN") == 0) {
            win = atoi(value);
        } else if (strcmp(key, "LOSE") == 0) {
            lose = atoi(value);
        }
    }

    // Close the file
    fclose(file);

    printf("HELPERS: %d\n", helperNum);
    printf("SPIES: %d\n", spiesNum);
    printf("WIN: %d\n", win);
    printf("LOSE: %d\n", lose);

    int numProcesses = helperNum + spiesNum + 3;
    pid_t pid;
    child_pids = (int *) malloc(numProcesses * sizeof(int));
    int child_count = 0;
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    struct sigaction sa1 = {0};
    sa1.sa_handler = &signal_handler2;
    sigaction(SIGUSR2, &sa1, NULL); // set up signal handler for SIGUSR1
    for (int i = 0; i < numProcesses; i++) {                                //forking all childs
        pid = fork();
        sleep(1);
        if (pid < 0) {
            perror("Fork failed\n");
            exit(1);
        } else if (pid == 0) {
            if (i == 0) {
                if (execl("./sender", "./sender", (char *) NULL) == -1) {
                    perror("Exec Error");
                    exit(-1);
                }
            } else if (i == 1) {
                //memset(shared_memory1, 0, sizeof(struct shared_data));
                if (execl("./master", "./master", (char *) NULL) == -1) {
                    perror("Exec Error");
                    exit(-1);
                }
            } else if (i == 2) {
                if (execl("./receiver", "./receiver", (char *) NULL) == -1) {
                    perror("Exec Error");
                    exit(-1);
                }
            } else if (i > 2 && i < numProcesses - helperNum) {
                char *i1 = (char *) malloc(sizeof(int));
                sprintf(i1, "%d", i - 3);

                printf("IM SPY PID: %d\n", getpid());
                if (execl("./spy", "./spy", i1, (char *) NULL) == -1) {
                    perror("Exec Error");
                    exit(-1);
                }
            } else {
                char *i1 = (char *) malloc(sizeof(int));
                sprintf(i1, "%d", i - (3 + spiesNum));

                char *i2 = (char *) malloc(sizeof(int));
                sprintf(i2, "%d", helperNum);

                if (execl("./helper", "./helper", i1, i2, (char *) NULL) == -1) {
                    perror("Exec Error");
                    exit(-1);
                }
            }
        }
        child_pids[child_count] = pid;              //saving processes pid for later use
        child_count++;
    }
    pid_t dis = fork();
    if (dis < 0) {
        perror("Fork failed\n");
        exit(1);
    } else if (dis == 0) {
        char *i1 = (char *) malloc(sizeof(int));
        sprintf(i1, "%d_%d_%d_%d", helperNum, spiesNum, win, lose);                         //openGL stuffs
        if (execl("./display", "./display", i1, (char *) NULL) == -1) {
            perror("Exec Error");
            exit(-1);
        }
    }
    int fd;
    char columns[3];
    sleep(2);
    fd = open(FIFO_NAME, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1)
        perror("Error reading FIFO file\n");                            //receiving the numEncode
    //printf("Number of columns3 %s:\n", columns);
    numEncode = atoi(columns);
    close(fd);
    unlink(FIFO_NAME);
    //sem_t *semaphores[numEncode];
    int fd1;
    char msg[3];
    sprintf(msg, "%d", numEncode);
    fd1 = open(FIFO_NAME1, O_WRONLY);
    if (fd1 == -1) {
        perror("open");
        exit(1);
    }
    for (int i = 0; i < helperNum; i++) {
        if (write(fd1, msg, strlen(msg) + 1) == -1) {
            perror("write");
            exit(1);
        }
        usleep(10000);
    }
    if (close(fd1) == -1) {
        perror("close");
        exit(1);
    }
    if (unlink(FIFO_NAME1) == -1) {
        perror("unlink");
        exit(1);
    }

    int fd2;
    char msg1[3];
    sprintf(msg1, "%d", numEncode);
    fd2 = open(FIFO_NAME2, O_WRONLY);
    if (fd2 == -1) {
        perror("open");
        exit(1);
    }
    if (write(fd2, msg1, strlen(msg1) + 1) == -1) {
        perror("write");
        exit(1);
    }
    if (close(fd2) == -1) {
        perror("close");
        exit(1);
    }
    if (unlink(FIFO_NAME2) == -1) {
        perror("unlink");
        exit(1);
    }

    int fd3;
    char msg3[3];
    sprintf(msg3, "%d", numEncode);
    fd3 = open(FIFO_NAME3, O_WRONLY);
    if (fd3 == -1) {
        perror("open");
        exit(1);
    }
    for (int i = 0; i < spiesNum; i++) {
        if (write(fd3, msg3, strlen(msg3) + 1) == -1) {
            perror("write");
            exit(1);
        }
        usleep(10000);
    }
    if (close(fd3) == -1) {
        perror("close");
        exit(1);
    }
    if (unlink(FIFO_NAME3) == -1) {
        perror("unlink");
        exit(1);
    }

    int fd4;
    char msg4[3];
    sprintf(msg4, "%d", numEncode);
    fd4 = open(FIFO_NAME4, O_WRONLY);
    if (fd4 == -1) {
        perror("open");
        exit(1);
    }
    if (write(fd4, msg4, strlen(msg4) + 1) == -1) {
        perror("write");
        exit(1);
    }
    if (close(fd4) == -1) {
        perror("close");
        exit(1);
    }
    if (unlink(FIFO_NAME4) == -1) {
        perror("unlink");
        exit(1);
    }
    //sending numencode to other procs
    int fd5;
    char msg5[3];
    sprintf(msg5, "%d", numEncode);
    fd5 = open(FIFO_NAME5, O_WRONLY);
    if (fd5 == -1) {
        perror("open");
        exit(1);
    }
    if (write(fd5, msg5, strlen(msg5) + 1) == -1) {
        perror("write");
        exit(1);
    }
    if (close(fd5) == -1) {
        perror("close");
        exit(1);
    }
    if (unlink(FIFO_NAME5) == -1) {
        perror("unlink");
        exit(1);
    }
    /*   for (int i = 0; i < numEncode; i++) {
           char semaphore_name[64];
           sprintf(semaphore_name, "%s%d", SEMAPHORE_PREFIX, i);
           semaphores[i] = sem_open(semaphore_name, O_CREAT, 0666, 1);
       }*/
    fflush(stdout);
    shared_memory3->num[2] = roundNum;
    while (receiverScore < win && spyScore < lose) {
        kill(child_pids[0], SIGUSR1);
        for (int i = 0; i < numEncode; i++) {
            sem_wait(semaphoreEncodeWait);
        }
        printf("IM HERE\n");
        fflush(stdout);

        kill(child_pids[1], SIGUSR1);                           //turning master on

        for (int i = numProcesses - helperNum; i < child_count; i++) {                 //turning on helpers
            kill(child_pids[i], SIGUSR1);
            usleep(i * 10000);
        }
        kill(child_pids[2], SIGUSR1);                                       //turning on receiver
        for (int i = 3; i < numProcesses - helperNum; i++) {                          //turning on spies
            printf("im spy kill %d\n", i);
            kill(child_pids[i], SIGUSR1);
            usleep(i * 10000);
        }
        while (1) {
            if (spyWin == 1) {
                printf("Spy Win\n");
                fflush(stdout);
                spyWin = 0;
                receiverWin = 0;
                spyScore++;
                shared_memory3->num[0] = spyScore;
                for (int i = numProcesses - helperNum; i < child_count; i++) {                   //turning off helpers
                    kill(child_pids[i], SIGUSR2);
                }
                kill(child_pids[2], SIGUSR2);                                       //turning off receiver
                for (int i = 3; i < numProcesses - helperNum; i++) {                            //turning off spies
                    kill(child_pids[i], SIGUSR2);
                }
                kill(child_pids[1], SIGUSR2);                                       //turning off master and sender
                kill(child_pids[2], SIGUSR2);
                break;
            } else if (receiverWin == 1) {
                printf("Receiver Win\n");
                fflush(stdout);
                receiverWin = 0;
                spyWin = 0;
                receiverScore++;
                shared_memory3->num[1] = receiverScore;
                for (int i = numProcesses - helperNum; i < child_count; i++) {                  //turning off helpers
                    kill(child_pids[i], SIGUSR2);
                }
                kill(child_pids[2], SIGUSR2);                                       //turning off receiver
                for (int i = 3; i < numProcesses - helperNum; i++) {                       //turning off spies
                    kill(child_pids[i], SIGUSR2);
                }
                kill(child_pids[1], SIGUSR2);
                kill(child_pids[2], SIGUSR2);
                break;
            } else {
                sleep(1);
            }
        }
        sleep(1);
        for (int i = numProcesses - helperNum; i < child_count; i++) {                 //turning off helpers
            kill(child_pids[i], SIGUSR2);
        }
        kill(child_pids[2], SIGUSR2);                                       //turning off receiver
        for (int i = 3; i < numProcesses - helperNum; i++) {                         //turning off spies
            kill(child_pids[i], SIGUSR2);
        }
        shared_memory3->num[2] = roundNum;                                              //openGL stuffs
        roundNum++;
        sleep(5);
    }
    if (sem_close(semaphoreEncode) == -1) {
        perror("sem_close");
        exit(1);
    }
    if (sem_unlink("/write_semaphore") == -1) {
        perror("sem_unlink");
        exit(1);
    }
    // Detach from the shared memory segment
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmdt(shared_memory1) == -1) {                  //closing and deataching and removing everything
        perror("shmdt");
        exit(1);
    }

    if (shmdt(shared_memory2) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmdt(shared_memory3) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmdt(shared_memory4) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmdt(shared_memory5) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Unlink FIFO_NAME5
    /*    for (int i = 0; i < numEncode; i++) {
        char semaphore_name[64];
        sprintf(semaphore_name, "%s%d", SEMAPHORE_PREFIX, i);
        sem_close(semaphores[i]);
        sem_unlink(semaphore_name);
    }*/
    key_t keyq = 6565;
    int msgid = msgget(keyq, 0);
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        // Handle error
    }
    kill(child_pids[0], SIGUSR2);
    for (int i = 1; i < numProcesses; i++) {
        kill(child_pids[i], SIGKILL);
    }
    kill(dis, SIGKILL);                                                              //thanks guys its time to die
    if (spyScore == lose) {
        printf("Master Spy Won\n");
    } else if (win == receiverScore) {
        printf("Receiver Won\n");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    if (shmctl(shmid1, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    if (shmctl(shmid2, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    if (shmctl(shmid3, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    if (shmctl(shmid4, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    if (shmctl(shmid5, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }
    sleep(7);
    free(child_pids);                               //dont forget to free
    return 0;
}