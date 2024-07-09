#ifndef LISTAPLAYER_H
#define LISTAPLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>

#include "CodaThreads.h"

#define BUFFER_SIZE 1024

typedef struct n
{
    pid_t pid_client;
    char val[BUFFER_SIZE];
    int client_socket;
    int punteggio;
    Coda paroleInserite;
    struct n *next;

} Player;

typedef Player *Lista;

void print(Lista l);

Player *addClient(pid_t pid, int client_sock, Lista *l);

int ricercaParola(char x[], Lista l);

void deleted(pid_t x, Lista *L);

Player *ricercaNodo(pid_t, Lista);

int is_process_running(pid_t pid) ;

void wait_for_process(pid_t pid) ;

void destroy(Lista *L);

#endif  //LISTAPLAYER_H

