#ifndef CODA_H
#define CODA_H

typedef struct P
{
    char *val;
    struct P *next;
} Nodo;

typedef struct
{
    Nodo *head;
    Nodo *tail;
} Coda;

void CodaInit(Coda * c);
void push(char * val, Coda * c);
char *pop(Coda * c);
int searchWord(Coda * c, char * word);
void clearQueue(Coda * c);
void destroyQueue(Coda * c);
void printCoda(Coda * c);
int contaElementiCoda(Coda * c);
int estraiPunteggio(char * val);
void scambiaValori(Nodo * a, Nodo * b);
void ordinaCoda(Coda * coda);
char *creaClassifica(Coda * c);
char *creaBacheca(Coda * c);

#endif // CODA_H

