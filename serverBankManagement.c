//
// Created by nomegigi-port on 23/04/25.
//
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcpmtHeader.h"

extern pthread_mutex_t structAccess;
extern bankAccount bank_account;


void initBankAccount(const char *name, const char *surname) {
    bank_account.name = strdup(name);
    bank_account.surname = strdup(surname);

    bank_account.movementList = malloc(sizeof(listFoot));
    if (!bank_account.movementList) {
        perror("Failed to allocate movementList");
        exit(EXIT_FAILURE);
    }
    bank_account.movementList->foot = NULL;
}

void movADD(movement *mov) {
    pthread_mutex_lock(&structAccess);
    mov->prev = bank_account.movementList->foot;
    bank_account.movementList->foot = mov;
    if (mov->prev != NULL) {
        mov->id = mov->prev->id + 1;
    }
    pthread_mutex_unlock(&structAccess);
}

void movDEL(int fd, int id) {
    pthread_mutex_lock(&structAccess);

    if (bank_account.movementList == NULL || bank_account.movementList->foot == NULL) {
        sendString(fd, "List has no elements\n");
        pthread_mutex_unlock(&structAccess);
        return;
    }

    movement *current = bank_account.movementList->foot;
    movement *prev = NULL;
    movement *adder = bank_account.movementList->foot;

    while (current != NULL) {
        if (current->id == id) {
            if (prev == NULL) {
                bank_account.movementList->foot = current->prev;
            } else {
                prev->prev = current->prev;

                while (adder != NULL) {
                    if (adder->id == id-1) {
                        break;
                    }
                    adder->id = adder->id - 1;
                    adder = adder->prev;

                }
            }

            free(current);
            sendString(fd, "Movement deleted successfully.\n");
            pthread_mutex_unlock(&structAccess);
            return;
        }

        prev = current;
        current = current->prev;
    }

    sendString(fd, "ID not found.\n");
    pthread_mutex_unlock(&structAccess);
}


void movUPDATE(int fd, int id_to_Change, movement *mov) {
    pthread_mutex_lock(&structAccess);
    movement *temp = bank_account.movementList->foot;

    while (temp != NULL) {
        if (temp->id == id_to_Change) {
            strcpy(temp->reason, mov->reason);
            temp->amount = mov->amount;
            strcpy(temp->recipient, mov->recipient);
            free(mov);
            pthread_mutex_unlock(&structAccess);
            return;
        }
        temp = temp->prev;
    }
    sendString(fd, "\nID not found!\n");
    pthread_mutex_unlock(&structAccess);
}

void movLIST(int fd) {
    pthread_mutex_lock(&structAccess);

    sendString(fd, "\nThis is the bank account of: ");
    sendString(fd, bank_account.name);
    sendString(fd, " ");
    sendString(fd, bank_account.surname);
    sendString(fd, "\n");

    //printf("\nQuesto è il conto di: %s %s", bank_account.name, bank_account.surname);

    if (bank_account.movementList == NULL || bank_account.movementList->foot == NULL) {
        sendString(fd, "No movement available");
      //  printf("\nList has no elements");
        pthread_mutex_unlock(&structAccess);
        return;
    }
    movement* temp = bank_account.movementList->foot;

    while (temp != NULL) {
        char idStr[128], amountStr[128];
        snprintf(idStr, sizeof(idStr), "%d", temp->id);
        snprintf(amountStr, sizeof(amountStr), "%f", temp->amount);

        sendString(fd,"\n#############\n Id = ");
        sendString(fd, idStr);
        sendString(fd, "\n Amount: ");
        sendString(fd, amountStr);
        sendString(fd, "\n Recipient: ");
        sendString(fd, temp->recipient);
        sendString(fd, "\n Reason: ");
        sendString(fd, temp->reason);
        sendString(fd,"\n#############\n");

        temp = temp->prev;
    }
    pthread_mutex_unlock(&structAccess);
}
