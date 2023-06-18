#include <stdio.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shared.h"

#define FIFO_NAME5 "myfifo5"
#define MAX_VALUES 100

int spyScore = 0, recScore = 0, roundNum = 0, numEncode = 0, helpers = 0, spies = 0, win = 0, lose = 0;
char leftData[MAX_VALUES][100000], middleData[MAX_VALUES][100000], rightData[MAX_VALUES][100000], spiesMsg[MAX_VALUES][100000];
char roundScoreString[30], recScoreString[30], spyScoreString[30];
int swappers[MAX_VALUES][2];


void drawText(char *text, float x, float y, float size) {
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; ++i) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void renderText(float x, float y, void *font, char *string) {
    glRasterPos2f(x, y);

    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(font, string[i]);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate the container dimensions based on the number of strings
    float containerHeight = 0.8f / numEncode;
    float containerWidth = 0.8f;

    // Calculate the maximum text width based on the longest word
    float maxLength = 0.0f;
    for (int i = 0; i < numEncode; i++) {
        float length = strlen(leftData[i]);
        if (length > maxLength) {
            maxLength = length;
        }
    }

    // Adjust the text size based on the length of the longest word
    float LtextSize = containerHeight * 0.4f;
    float textSize = containerHeight * 0.4f;
    if (maxLength > 8) {
        textSize = (15.0f * maxLength * 0.01);
    }

    // Calculate the vertical offset for centering the containers
    float yOffset = (0.5f - numEncode * containerHeight) / 2.0f;
    // Draw the left container label
    glColor3f(1.0f, 0.0f, 0.0f);
    float labelWidth = 0.1f * strlen("Master Spy") * LtextSize; // Adjust the scaling factor (0.1f) as needed
    float labelX = -1.0f + containerWidth / 2.0f - labelWidth / 2.0f - 0.2f; // Adjust the shift (-0.2f) as needed
    float labelY = 0.95f;
    drawText("Master Spy", labelX, labelY, textSize);

    // Draw the middle container label
    glColor3f(0.0f, 1.0f, 0.0f);
    labelWidth = 0.1f * strlen("Messages") * LtextSize; // Adjust the scaling factor (0.1f) as needed
    labelX = -labelWidth / 2.0f;
    labelY = 0.95f;
    drawText("Messages", labelX, labelY, LtextSize);

    // Draw the right container label
    glColor3f(0.0f, 0.0f, 1.0f);
    labelWidth = 0.1f * strlen("Receiver") * LtextSize; // Adjust the scaling factor (0.1f) as needed
    labelX = containerWidth / 2.0f - labelWidth / 2.0f + 0.3f; // Adjust the shift (0.3f) as needed
    labelY = 0.95f;
    drawText("Receiver", labelX, labelY, LtextSize);

    // Draw the left containers and text
    for (int i = 0; i < numEncode; i++) {
        // Calculate the position of the current container
        float containerY = 1.0f - (i + 1) * containerHeight + yOffset;

        // Set the color of the container
        glColor3f(1.0f, 0.0f, 0.0f);

        // Draw the container
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, containerY);
        glVertex2f(-1.0f + containerWidth, containerY);
        glVertex2f(-1.0f + containerWidth, containerY + containerHeight);
        glVertex2f(-1.0f, containerY + containerHeight);
        glEnd();

        // Set the position and size for the text
        float textX = -1.0f + 0.05f;
        float textY = containerY + containerHeight / 2.0f;

        // Set the color for the text to match the container
        glColor3f(1.0f, 1.0f, 1.0f);

        // Draw the text
        drawText(leftData[i], textX, textY, textSize);
    }

    // Draw the middle containers and text
    for (int i = 0; i < numEncode; i++) {
        // Calculate the position of the current container
        float containerY = 1.0f - (i + 1) * containerHeight + yOffset;

        // Set the color of the container
        glColor3f(0.0f, 1.0f, 0.0f);

        // Draw the container
        glBegin(GL_QUADS);
        glVertex2f(-containerWidth / 2, containerY);
        glVertex2f(-containerWidth / 2 + containerWidth, containerY);
        glVertex2f(-containerWidth / 2 + containerWidth, containerY + containerHeight);
        glVertex2f(-containerWidth / 2, containerY + containerHeight);
        glEnd();

        // Set the position and size for the text
        float textX = -containerWidth / 2 + 0.05f;
        float textY = containerY + containerHeight / 2.0f;

        // Set the color for the text to match the container
        glColor3f(0.0f, 0.0f, 0.0f);

        // Draw the text
        drawText(middleData[i], textX, textY, textSize);
    }

    // Draw the right containers and text
    for (int i = 0; i < numEncode; i++) {
        // Calculate the position of the current container
        float containerY = 1.0f - (i + 1) * containerHeight + yOffset;

        // Set the color of the container
        glColor3f(0.0f, 0.0f, 1.0f);

        // Draw the container
        glBegin(GL_QUADS);
        glVertex2f(containerWidth / 2, containerY);
        glVertex2f(containerWidth / 2 + containerWidth, containerY);
        glVertex2f(containerWidth / 2 + containerWidth, containerY + containerHeight);
        glVertex2f(containerWidth / 2, containerY + containerHeight);
        glEnd();

        // Set the position and size for the text
        float textX = containerWidth / 2 + 0.05f;
        float textY = containerY + containerHeight / 2.0f;

        // Set the color for the text to match the container
        glColor3f(1.0f, 1.0f, 1.0f);

        // Draw the text
        drawText(rightData[i], textX, textY, textSize);
    }

    glColor3f(1.0f, 0.0f, 0.0f);
    labelWidth = 0.1f * strlen("Spies") * LtextSize; // Adjust the scaling factor (0.1f) as needed
    labelX = -1.0f + containerWidth / 2.0f - labelWidth / 2.0f - 0.2f; // Adjust the shift (-0.2f) as needed
    labelY = -0.1f;
    drawText("Spies", labelX, labelY, textSize);

    if (spies > 10) {
        containerHeight = 0.8f / (spies + 7);
        containerWidth = 0.8f;

        textSize = containerHeight * 0.4f;
        if (maxLength > 8) {
            textSize = (15.0f * maxLength * 0.01);
        }
    }

    // Calculate the maximum text width based on the longest word
    for (int i = 0; i < spies; i++) {
        // Calculate the position of the current container
        float containerY = 0.0f - (i + 1) * containerHeight + yOffset;

        // Set the color of the container
        glColor3f(1.0f, 0.0f, 0.0f);

        // Draw the container
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, containerY);
        glVertex2f(-1.0f + containerWidth, containerY);
        glVertex2f(-1.0f + containerWidth, containerY + containerHeight);
        glVertex2f(-1.0f, containerY + containerHeight);
        glEnd();

        // Set the position and size for the text
        float textX = -1.0f + 0.05f;
        float textY = containerY + containerHeight / 2.0f;


        // Draw the text
        glColor3f(1.0f, 1.0f, 1.0f);
        char *i2 = (char *) malloc(15 + sizeof(spiesMsg[i]));
        sprintf(i2, "Spy %d sent message: %s", i, spiesMsg[i]);
        drawText(i2, textX, textY, textSize);
    }

    glColor3f(0.0f, 0.0f, 1.0f);
    labelWidth = 0.1f * strlen("Helpers") * LtextSize; // Adjust the scaling factor (0.1f) as needed
    labelX = containerWidth / 2.0f - labelWidth / 2.0f + 0.3f; // Adjust the shift (0.3f) as needed
    labelY = -0.1f;
    drawText("Helpers", labelX, labelY, LtextSize);

    containerHeight = 0.8f / (float) helpers;
    containerWidth = 0.8f;

    // Calculate the maximum text width based on the longest word
    LtextSize = containerHeight * 0.4f;
    textSize = containerHeight * 0.4f;
    for (int i = 0; i < helpers; i++) {// Calculate the position of the current container
        float containerY = 0.0f - (i + 1) * containerHeight + yOffset;

        // Set the color of the container
        glColor3f(0.0f, 0.0f, 1.0f);

        // Draw the container
        glBegin(GL_QUADS);
        glVertex2f(containerWidth / 2, containerY);
        glVertex2f(containerWidth / 2 + containerWidth, containerY);
        glVertex2f(containerWidth / 2 + containerWidth, containerY + containerHeight);
        glVertex2f(containerWidth / 2, containerY + containerHeight);
        glEnd();

        // Set the position and size for the text
        float textX = containerWidth / 2 + 0.05f;
        float textY = containerY + containerHeight / 2.0f;

        // Draw the text
        glColor3f(1.0f, 1.0f, 1.0f);
        char *i2 = (char *) malloc(60 * sizeof(char));
        sprintf(i2, "Helper %d swapped between column %d and %d", i, swappers[i][0], swappers[i][1]);
        drawText(i2, textX, textY, textSize);
    }
    containerHeight = 0.8f / numEncode;
    containerWidth = 0.8f;
    float spyScoresContainerY = -0.8f;
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, spyScoresContainerY);
    glVertex2f(-1.0f + containerWidth, spyScoresContainerY);
    glVertex2f(-1.0f + containerWidth, spyScoresContainerY + containerHeight);
    glVertex2f(-1.0f, spyScoresContainerY + containerHeight);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(spyScoreString, -1.0f + 0.05f, spyScoresContainerY + containerHeight / 2.0f,
             LtextSize);// Assuming the spyScore can fit in 10 characters
    // Receiver score container
    float receiverScoreContainerY = -0.8f - containerHeight;
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, receiverScoreContainerY);
    glVertex2f(-1.0f + containerWidth, receiverScoreContainerY);
    glVertex2f(-1.0f + containerWidth, receiverScoreContainerY + containerHeight);
    glVertex2f(-1.0f, receiverScoreContainerY + containerHeight);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(recScoreString, -1.0f + 0.05f, receiverScoreContainerY + containerHeight / 2.0f, LtextSize);
// Assuming the spyScore can fit in 10 characters
    // Round container
    float roundContainerY = -0.8f - 2 * containerHeight;
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, roundContainerY);
    glVertex2f(-1.0f + containerWidth, roundContainerY);
    glVertex2f(-1.0f + containerWidth, roundContainerY + containerHeight);
    glVertex2f(-1.0f, roundContainerY + containerHeight);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(roundScoreString, -1.0f + 0.05f, roundContainerY + containerHeight / 2.0f, LtextSize);

    glutSwapBuffers();
    glFlush();
}


void updateData() {
    key_t key = 141414;  // Generate the same key as in Process 1
    int shmid = shmget(key, sizeof(struct shared_data), 0);  // Get the existing shared memory segment
    struct shared_data *shared_memory = (struct shared_data *) shmat(shmid, NULL, 0);  // Attach shared memory segment

    key_t key1 = 123432;
    int shmid1 = shmget(key1, sizeof(struct shared_data), 0);  // Get the existing shared memory segment
    struct shared_data *shared_memory1 = (struct shared_data *) shmat(shmid1, NULL, 0);  // Attach shared memory segment
    // Update the left, middle, and right data arrays

    key_t key2 = 1234321;
    int shmid2 = shmget(key2, sizeof(struct shared_data), IPC_CREAT | 0666);  // Create second shared memory segment
    struct shared_data *shared_memory2 = (struct shared_data *) shmat(shmid2, NULL,
                                                                      0);  // Attach second shared memory segment

    key_t key3 = 12343211;
    int shmid3 = shmget(key3, sizeof(struct shared_data1), 0);  // Get the shared memory segment ID
    struct shared_data1 *shared_memory3 = (struct shared_data1 *) shmat(shmid3, NULL,
                                                                        0);  // Attach second shared memory segment

    key_t key4 = 1234321123;
    int shmid4 = shmget(key4, sizeof(struct shared_data2), IPC_CREAT | 0666);  // Create second shared memory segment
    struct shared_data2 *shared_memory4 = (struct shared_data2 *) shmat(shmid4, NULL,
                                                                        0);  // Attach second shared memory segment

    key_t key5 = 1234321321;
    int shmid5 = shmget(key5, sizeof(struct shared_data3), IPC_CREAT | 0666);  // Create second shared memory segment
    struct shared_data3 *shared_memory5 = (struct shared_data3 *) shmat(shmid5, NULL,
                                                                        0);  // Attach second shared memory segment


    for (int i = 0; i < numEncode; i++) {
        strcpy(leftData[i], shared_memory1->value[i]);
        strcpy(middleData[i], shared_memory->value[i]);
        strcpy(rightData[i], shared_memory2->value[i]);
        swappers[i][0] = shared_memory4->swappers[i][0];
        swappers[i][1] = shared_memory4->swappers[i][1];
    }

    for (int i = 0; i < helpers; i++) {
        swappers[i][0] = shared_memory4->swappers[i][0];
        swappers[i][1] = shared_memory4->swappers[i][1];
    }

    for (int i = 0; i < spies; i++) {
        strcpy(spiesMsg[i], shared_memory5->spies[i]);
    }

    spyScore = shared_memory3->num[0];
    recScore = shared_memory3->num[1];
    roundNum = shared_memory3->num[2];
    sprintf(spyScoreString, "Spy Score: %d", spyScore);
    sprintf(recScoreString, "Receiver Score: %d", recScore);
    sprintf(roundScoreString, "Round: %d", roundNum);

    // Detach the shared memory
    shmdt(shared_memory);
}

void display1() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Declare the winner
    glColor3f(0, 1, 0);
    if (win == recScore)
        renderText(-0.2f, 0, GLUT_BITMAP_HELVETICA_18, "The winner is RECEIVER");
    else if (lose == spyScore)
        renderText(-0.2f, 0, GLUT_BITMAP_HELVETICA_18, "The winner is MASTER SPY");
    else
        renderText(-0.05F, 0, GLUT_BITMAP_HELVETICA_18, "TIE");

    glutSwapBuffers();
    glFlush();
    glutLeaveMainLoop();
}

void timer(int) {
    updateData();

    // Redraw the display
    glutPostRedisplay();

    // Call the timer function again after a certain interval
    if (win == recScore || lose == spyScore)
        glutTimerFunc(5000, display1, 0);
    else
        glutTimerFunc(1000 / 60, timer, 0);
}

int main(int argc, char **argv) {
    // Read the numEncode value from the FIFO
    int fd;
    char columns[3];
    fd = open(FIFO_NAME5, O_RDONLY);
    if (read(fd, &columns, sizeof(columns)) == -1) {
        perror("Error reading FIFO file");
        exit(1);
    }
    close(fd);
    unlink(FIFO_NAME5);
    numEncode = atoi(columns);

    sscanf(argv[1], "%d_%d_%d_%d", &helpers, &spies, &win, &lose);

    printf("Display: numEncode %d, helpers %d, spies %d, win %d, lose %d\n", numEncode, helpers, spies, win, lose);
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Shared Memory User Interface");

    // Set the display function
    glutDisplayFunc(display);

    // Set the clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Start the timer to continuously update the data
    glutTimerFunc(0, timer, 0);

    // Start the GLUT main loop
    glutMainLoop();

    return 0;
}