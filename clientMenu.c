#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "tcpmtHeader.h"

extern int connectedClients;
extern pthread_mutex_t clientlock;
extern pthread_mutex_t bank_account;

// Updates client connection count safely
void updateCounter(char opt) {
    pthread_mutex_lock(&clientlock);
    if (opt == '+') {
        connectedClients++;
    } else if (opt == '-') {
        connectedClients--;
    }
    pthread_mutex_unlock(&clientlock);
}

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void sendString(int fd, const char *str) {
    send(fd, str, strlen(str), 0);
}

ssize_t readLine(int fd, char *buffer, size_t maxLen) {
    size_t i = 0;
    char c;
    while (i < maxLen - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c == '\r') continue;

        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}

int readChar(int fd, char *c) {
    ssize_t n = read(fd, c, 1);
    if (n <= 0) return -1;

    char discard;
    while (read(fd, &discard, 1) == 1) {
        if (discard == '\n') break;
    }
    return 1;
}

int readInt(int fd) {
    char idbuf[64];
    readLine(fd, idbuf, sizeof(idbuf));
    return atoi(idbuf);
}

movement *movementGenerator(int fd) {
    char recipientBuf[BUFFER_SIZE];
    char paymentReasonBuf[BUFFER_SIZE];
    char amountBuf[BUFFER_SIZE];

    sendString(fd, "\nType name and surname of the recipient:\n");
    if (readLine(fd, recipientBuf, sizeof(recipientBuf)) <= 0) {
        sendString(fd, "Error reading recipient.\n");
        return NULL;
    }

    sendString(fd, "\nInsert payment amount:\n");
    if (readLine(fd, amountBuf, sizeof(amountBuf)) <= 0) {
        sendString(fd, "Error reading amount.\n");
        return NULL;
    }
    double convertedAmount = strtod(amountBuf, NULL);

    sendString(fd, "\nInsert payment reason:\n");
    if (readLine(fd, paymentReasonBuf, sizeof(paymentReasonBuf)) <= 0) {
        sendString(fd, "Error reading payment reason.\n");
        return NULL;
    }

    movement *generatedMov = (movement *) malloc(sizeof(movement));
    if (!generatedMov) {
        sendString(fd, "Memory allocation error.\n");
        return NULL;
    }
    memset(generatedMov, 0, sizeof(movement));

    generatedMov->amount = convertedAmount;
    strncpy(generatedMov->recipient, recipientBuf, BUFFER_SIZE - 1);
    generatedMov->recipient[BUFFER_SIZE - 1] = '\0';
    strncpy(generatedMov->reason, paymentReasonBuf, BUFFER_SIZE - 1);
    generatedMov->reason[BUFFER_SIZE - 1] = '\0';

    return generatedMov;
}

void mainMenu(int fd) {
    char *mainMenuStr =
            "\n\nSelect an operation:\n"
            " (a) ADD\n"
            " (d) DEL\n"
            " (u) UPDATE\n"
            " (l) LIST\n"
            " (e) exit\n> ";

    char optionBuf;

    while (1) {
        sendString(fd, mainMenuStr);

        ssize_t len = readChar(fd, &optionBuf);
        if (len <= 0) {
            continue;
        }

        switch (optionBuf) {
            case 'a':
                sendString(fd, "\nYou chose ADD\n");
                movement *toAdd = movementGenerator(fd);

                if (toAdd != NULL) {
                    movADD(toAdd);
                    sendString(fd, "\nMovement added successfully.\n");
                } else {
                    sendString(fd, "\nCould not add movement.\n");
                }

                break;

            case 'd':
                sendString(fd, "\nYou chose DEL\n");

                sendString(fd, "\nSelect a movement to delete\n");
                int toDelete = readInt(fd);

                if (toDelete < 0) {
                    sendString(fd, "Invalid ID.\n");
                }

                movDEL(fd, toDelete);
                break;

            case 'u':
                sendString(fd, "\nYou chose UPDATE\n");
                sendString(fd, "\nInsert id to change\n");

                int toUpdate = readInt(fd);
                movement *updated = movementGenerator(fd);

                if (updated != NULL) {
                    movUPDATE(fd, toUpdate, updated);
                    sendString(fd, "\nMovement successfully updated.\n");

                } else {
                    sendString(fd, "\nCould not update movement.\n");
                }
                break;
            case 'l':
                sendString(fd, "\nYou chose LIST\n");
                movLIST(fd);
                break;
            case 'e':
                sendString(fd, "\nExiting. Goodbye!\n");
                return;
            default:
                sendString(fd, "\nUnknown option, try again.\n");
                break;
        }
    }
}

void *clientHandler(int *fd) {
    int clientfd = *fd;
    free(fd);

    printf("\nConnected client with FD: %d\n", clientfd);
    fflush(stdout);

    mainMenu(clientfd);

    printf("\nClient disconnected\n");
    updateCounter('-');

    fflush(stdout);
    close(clientfd);
    pthread_exit(NULL);
}
