//
// Created by nomegigi-port on 23/04/25.
//
#include <string.h>
#ifndef TCPMTHEADER_H
#define TCPMTHEADER_H

#define PORT 8080
#define BUFFER_SIZE 256
#define MAX_NUM 10


#define MAX_OPERATIONS_QUEUE 16


typedef struct movement {
    int id;
    double amount;
    char recipient[BUFFER_SIZE];
    char reason[BUFFER_SIZE];
    struct movement* prev;
}movement;

typedef struct listFoot {
    struct movement* foot;
}listFoot;


typedef struct bankAccount {
    char* name;
    char* surname;
    struct listFoot* movementList;
}bankAccount;


void initBankAccount(const char *name, const char *surname);

void movADD(movement *mov);
void movDEL(int fd, int id);
void movUPDATE(int fd, int id_to_Change, movement *mov);
void movLIST(int fd);

void updateCounter(char opt);
void *clientHandler(int *fd);

void sendString(int fd, const char* str);
#endif //TCPMTHEADER_H