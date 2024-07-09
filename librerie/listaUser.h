#ifndef LISTAUSER_H
#define LISTAUSER_H

//lista concatenata in cui salveremo gli utenti già registrati

typedef struct u
{
    char * nomeUtente;
    struct u *next;
} Registrato;

typedef Registrato * Registrati;

int ricercaUtente(char x[], Registrati p);
void printUsers(Registrati p);
Registrato * registraUtente(char * nomeUtente, Registrati * l);
void cancellaUtente(char * x, Registrati *L);

#endif