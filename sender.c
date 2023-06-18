#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "shared.h"

#define FIFO_NAME "myfifo"
#define MAX_COLUMNS 100  // Maximum number of columns in the data
#define MAX_LENGTH 10000 // Maximum length of a column

pid_t *child_pids;
int numChild = 0, signalHandler = 0;
char columns[MAX_COLUMNS][MAX_LENGTH];  // Array to store the columns

void sigusr2_handler() {
    signalHandler = 2;
}

void toColumns() {
    FILE *inputFile, *outputFile;
    char line[MAX_LENGTH];
    int column = 1;
    int maxColumns = 0;

    // Open the input file for reading
    inputFile = fopen("sender.txt", "r");
    if (inputFile == NULL) {
        printf("Failed to open the input file.\n");
        return;
    }

    // Open the output file for writing
    outputFile = fopen("columns.txt", "w");
    if (outputFile == NULL) {
        printf("Failed to create the output file.\n");
        fclose(inputFile);
        return;
    }

    // Determine the highest number of columns
    while (fgets(line, MAX_LENGTH, inputFile) != NULL) {
        char *word;
        int count = 0;

        word = strtok(line, " "); // Split the line into words using the blank character as the delimiter

        while (word != NULL) {
            count++;
            word = strtok(NULL, " "); // Get the next word
        }

        if (count > maxColumns) {
            maxColumns = count;
        }
    }

    // Reset the file pointer to the beginning of the input file
    fseek(inputFile, 0, SEEK_SET);

    // Read and process each line of the input file
    while (fgets(line, MAX_LENGTH, inputFile) != NULL) {
        char *word;
        int count = 0;

        word = strtok(line, " "); // Split the line into words using the blank character as the delimiter

        // Process each word
        while (word != NULL) {
            // Check if the word ends with '\n' and remove it
            size_t len = strlen(word);
            if (len > 0 && word[len - 1] == '\n') {
                word[len - 1] = '\0';
                if (word[len - 1] == '\r')
                    word[len - 1] = '\0';
            }
            // Write the word to the output file in the desired format
            fprintf(outputFile, "%-15s", word);

            column++;
            count++;
            word = strtok(NULL, " "); // Get the next word
        }

        // Add "alright" to fill remaining columns on the line
        while (count < maxColumns) {
            fprintf(outputFile, "%-15s", "alright");
            column++;
            count++;
        }
        if (column > numChild)
            numChild = column - 1;

        column = 1; // Reset the column counter for the new line
        fprintf(outputFile, "\n"); // Add a newline character to move to the next line
    }

    // Close the files
    fclose(inputFile);
    fclose(outputFile);
}

void splitting() {
    FILE *file = fopen("columns.txt", "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return;
    }

    char line[1000];

    for (int i = 0; i < numChild; i++) {
        strcpy(columns[i], "");  // Initialize each column as an empty string
    }

    while (fgets(line, sizeof(line), file)) {
        char *word = strtok(line, " \t");  // Split the line into words

        for (int i = 0; i < numChild && word != NULL; i++) {
            if (strlen(columns[i]) > 0) {
                strcat(columns[i], " ");  // Append a space before each subsequent word
            }
            strcat(columns[i], word);  // Append the word to the corresponding column
            word = strtok(NULL, " \t");
        }
    }

    fclose(file);
}

void signal_handler() {
    signalHandler = 1;
}

int main() {
    if (mkfifo(FIFO_NAME, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create file fifo");
            exit(1);
        }
    }
    struct sigaction sa_usr2 = {0};
    sa_usr2.sa_handler = &sigusr2_handler;
    sigaction(SIGUSR2, &sa_usr2, NULL); // set up signal handler for SIGUSR2
    child_pids = (int *) malloc(numChild * sizeof(int));
    int child_count = 0;
    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGUSR1, &sa, NULL); // set up signal handler for SIGUSR1
    toColumns();
    splitting();                                //just split the file
    int fd;
    char msg[3];
    sprintf(msg, "%d", numChild);
    fd = open(FIFO_NAME, O_WRONLY);
    write(fd, msg, strlen(msg) + 1);
    close(fd);
    //printf("I'm the sender the number of columns = %d\n", numChild);
    pid_t pid;
    for (int i = 0; i < numChild + 1; i++) {
        pid = fork();
        sleep(1);
        if (pid < 0) {
            perror("Fork failed\n");
            exit(1);
        } else if (pid == 0) {
            // child process
            char *i1 = (char *) malloc(sizeof(int));
            char *grandParentPID = (char *) malloc(sizeof(int));
            sprintf(i1, "%d", i + 1);
            sprintf(grandParentPID, "%d", getppid());
            //printf("ENCODER PID %d\n",getpid());                                                              //forking encoders
            fflush(stdout);
            if (execl("./encoders", "./encoders", i1, columns[i], grandParentPID, (char *) NULL) == -1) {
                perror("Exec Error");
                exit(-1);
            }
        }
        child_pids[child_count] = pid;              //saving processes pid for later use
        child_count++;
    }
    //  for (int x = 0; x < numChild; x++) {
    //   kill(child_pids[x], SIGUSR1);
    //}
    //kill(getppid(), SIGUSR1);
    signalHandler = 1;
//    sleep(10);
    while (1) {
        if (signalHandler == 2) {
            for (int i = 0; i < numChild; i++) {
                kill(child_pids[i], SIGKILL);
            }
            kill(getpid(), SIGKILL);
            signalHandler = 0;
        } else if (signalHandler ==
                   1) {                                        //signals encoders each run and kill them according to what i send
            for (int i = 0; i < numChild; i++) {
                kill(child_pids[i], SIGUSR1);
                //printf("IM Number %d",i);
                fflush(stdout);
                usleep(1000 * (i + 1));
            }
            signalHandler = 0;
        } else {
            sleep(1);
        }
    }
}