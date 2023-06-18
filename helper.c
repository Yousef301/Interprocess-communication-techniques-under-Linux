#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "shared.h"
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define SEMAPHORE_PREFIX "/my_semaphore"
#define FIFO_NAME1 "myfifo1"

int shouldContinue = 0;

void signal_handler() {                         //signal handlers from our parent to know when to work
    shouldContinue = 1;
}

void signal_handler2() {
    shouldContinue = 0;
}

int main(int argc, char *argv[]) {
    int helperNum = atoi(argv[1]);
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    struct sigaction sa1 = {0};
    sa1.sa_handler = &signal_handler2;
    sigaction(SIGUSR2, &sa1, NULL); // set up signal handler for SIGUSR1
    int fd;
    char columns[3];
    if (mkfifo(FIFO_NAME1, 0777) == -1) {                               //fifo to receive numColumns
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    fd = open(FIFO_NAME1, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1)
        perror("Error reading FIFO file\n");
    printf("Number of columns 1 %s:\n", columns);
    int numEncode = atoi(columns);                      //receive the number of cols from our parent
    close(fd);
    //sem_t *semaphores[numEncode];
/*    sem_t *semaphores[numEncode];
    for (int i = 0; i < numEncode; i++) {
        char semaphore_name[64];
        sprintf(semaphore_name, "%s%d", SEMAPHORE_PREFIX, i);
        semaphores[i] = sem_open(semaphore_name, O_CREAT, 0666, 1);
    }*/
    while (1) {
        if (shouldContinue == 1) {
            key_t key = 141414;
            int shmid = shmget(key, sizeof(struct shared_data), 0);
            if (shmid == -1) {
                perror("shmget");
                exit(1);
            }

            struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL,
                                                                             0);           //shared memory attach
            if (shared_memory == (struct shared_data *) (-1)) {
                perror("shmat");
                exit(1);
            }

            key_t key4 = 1234321123;
            int shmid4 = shmget(key4, sizeof(struct shared_data2), IPC_CREAT | 0666);           //openGL
            if (shmid4 == -1) {
                perror("shmget");
                exit(1);
            }

            struct shared_data2 *shared_memory4 = (struct shared_data2 *) shmat(shmid4, NULL, 0);           //openGL
            if (shared_memory4 == (struct shared_data2 *) (-1)) {
                perror("shmat");
                exit(1);
            }
            printf("Swapping...\n");
            fflush(stdout);
            srand(time(NULL) + getpid());
            int random_number1 = rand() % (numEncode);
            int random_number2 = rand() % (numEncode);
            shared_memory4->swappers[helperNum][0] = random_number1;
            shared_memory4->swappers[helperNum][1] = random_number2;                                    //thats OpenGL stuffs

            //sem_wait(semaphores[random_number1]);  // Wait for access to the semaphore
            //sem_wait(semaphores[random_number2]);  // Wait for access to the semaphore
            char temp[SHARED_MEMORY_SIZE];
            strcpy(temp, shared_memory->value[random_number1]);
            strcpy(shared_memory->value[random_number1],
                   shared_memory->value[random_number2]);             //swap positions
            strcpy(shared_memory->value[random_number2], temp);
            //sem_post(semaphores[random_number1]);  // Release the semaphore
            //sem_post(semaphores[random_number2]);  // Wait for access to the semaphore
            sleep(2);
            if (shmdt(shared_memory) == -1) {
                perror("shmdt");
            }

            if (shmdt(shared_memory4) == -1) {
                perror("shmdt");
            }
        } else {
            sleep(1);
        }
    }

}