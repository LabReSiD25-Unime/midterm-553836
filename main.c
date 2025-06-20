 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "tcpmtHeader.h"

///////////////////////////////////////////////////////////////////
pthread_mutex_t clientlock = PTHREAD_MUTEX_INITIALIZER;
int connectedClients = 0;
///////////////////////////////////////////////////////////////////
pthread_mutex_t structAccess = PTHREAD_MUTEX_INITIALIZER;

///////////////////////////////////////////////////////////////////
bankAccount bank_account;
//////////////////////////////////////////////////////////////////


int main(void) {

    initBankAccount("Nome", "Gigi");
    int sockserverfd, clientsockfd;
    struct sockaddr_in serveraddr, clientaddr[MAX_NUM];
    socklen_t clientaddr_len = sizeof(clientaddr[0]);
    pthread_t clientThreads[MAX_NUM];



    // Creazione server
    if ((sockserverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\nCan't create socket\n");
        exit(EXIT_FAILURE);
    }

    // Configurazione indirizzo
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    setsockopt(sockserverfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Binding alla porta
    int opt = 1;
    setsockopt(sockserverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sockserverfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("\nCan't bind to the port\n)");
        exit(EXIT_FAILURE);
    }

    // Porre in ascolto
    if (listen(sockserverfd, MAX_NUM) < 0) {
        perror("\nCan't listen to the socket\n);");
        exit(EXIT_FAILURE);
    }

    // Server Loop
    while (1) {
        if (connectedClients < MAX_NUM) {

            if (( clientsockfd = accept(sockserverfd, (struct sockaddr *) &clientaddr[connectedClients], &clientaddr_len)) < 0) {
                perror("\nCan't accept a connection\n);");
                continue;
            }

            int *clientfd = malloc(sizeof(int));
            *clientfd = clientsockfd;

            if (pthread_create(&clientThreads[connectedClients], NULL, (void*)clientHandler, clientfd) != 0) {
                perror("\nCan't create thread\n);");
                close(*clientfd);
                free(clientfd);
            }
            updateCounter('+');
        } else {
            printf("\nConnections full!, waiting for another thread to stop\n);");
            sleep(1);
        }
    }
    close(sockserverfd);
    return 0;
}
