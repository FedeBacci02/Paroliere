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

#include "listaUser.h"

/* viene cercato il nome utente all interno della lista */

int ricercaUtente(char x[], Registrati p)
{
    if (p== NULL)
        return 0;
    if (strcmp(x, p->nomeUtente) == 0)
        return 1;
    return ricercaUtente(x, p->next);
}

/* stampa tutti gli elementi di una lista, partendo dalla testa, ricorsivamente */

void printUsers(Registrati p)
{
    if (p == NULL)
        return;
    printf("%s -> ", p->nomeUtente);
    printUsers(p->next);
}

/* mantiene i dati dell'utente registrato non ancora cancellato nel server */

Registrato * registraUtente(char * nomeUtente, Registrati * l)
{
    if (*l == NULL)
    {
        Registrato *nuovo = (Registrato *)malloc(sizeof(Registrato));
        if (nuovo == NULL)
        {
            perror("Errore nell'allocazione della memoria");
            exit(EXIT_FAILURE);
        }
        
        nuovo->nomeUtente = strdup(nomeUtente);
        nuovo->next = *l;
        *l = nuovo;
        return nuovo;
    }
    else
    {
        return registraUtente(nomeUtente, &((*l)->next));
    }
}

/* toglie l'utente dalla lista nel caso in cui si fosse connesso */

void cancellaUtente(char * x, Registrati *L)
{
    Registrato *attuale = *L;
    Registrato *prec = NULL;

    while (attuale != NULL)
    {
        printf("x = %s, nomeUtente = %s",x,attuale->nomeUtente);
        if (strcmp(x,attuale->nomeUtente)==0)
        {
            // printf("Eliminando: %s\n", attuale->val);
            if (prec == NULL)
            {
                // Se l'elemento da eliminare è il primo nella lista
                *L = attuale->next;
            }
            else
            {
                // Se l'elemento da eliminare non è il primo
                prec->next = attuale->next;
            }
            free(attuale);
            printf("user rimosso");
            return;
        }
        else
        {
            prec = attuale;
            attuale = attuale->next;
        }
    }
    printf("user non trovato");
}