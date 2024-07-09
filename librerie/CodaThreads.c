#include "CodaThreads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void CodaInit(Coda * c){
    c->head=NULL;
    c->tail=NULL;
}

// inserimento in coda alla lista
void push(char *val, Coda *c)
{
    Nodo *nuovo = (Nodo *)malloc(sizeof(Nodo));
    if (nuovo == NULL)
    {
        perror("errore nell allocazioe di memoria");
        exit(EXIT_FAILURE);
    }
    nuovo->val = strdup(val);

    if (nuovo->val == NULL)
    {
        perror("errore nell allocazione di memoria nella stringa");
        free(nuovo);
        exit(EXIT_FAILURE);
    }

    nuovo->next = NULL;

    if (c->tail == NULL)
    {
        // coda vuota assegnamo la testa e alla coda il nuovo valore
        c->tail = nuovo;
        c->head = nuovo;
    }
    else
    {
        // altrimenti assegnamo il nuovo valore solo alla testa
        c->tail->next = nuovo;
        c->tail = nuovo;
    }
}

// eliminazione in testa alla coda
char *pop(Coda *c)
{
    // controllo coda vuota
    if (c->head == NULL)
        return NULL;

    // assegnazione del nuovo valore in testa
    Nodo *temp = c->head;
    char *word = temp->val;
    c->head = c->head->next;

    // se abbiamo eliminato tutti i valori assicuriamoci che tail punti a null
    if (c->head == NULL)
        c->tail = NULL;

    // eliminazione del temp
    free(temp);

    // return parola rimossa (utilizzato per i controlli)
    return word;
}

int searchWord(Coda *c, char *word)
{
    if (c->head == NULL)
        return 0;
    Nodo *temp = c->head;
    while (temp != NULL)
    {
        // parola trovata
        if (strcmp(temp->val, word) == 0)
            return 1;

        temp = temp->next;
    }
    // parola non trovata
    return 0;
}

// azzera la coda
void clearQueue(Coda *c)
{
    char *word;
    while (c->head != NULL)
    {
        word = pop(c);
        free(word);
    }
}

// distrugge la coda creata
void destroyQueue(Coda *c)
{
    clearQueue(c);
    free(c);
}

void printCoda(Coda *c)
{

    if (c->head == NULL)
        return;
    Nodo *temp = c->head;
    while (temp != NULL)
    {
        printf("val: %s", temp->val);
        temp = temp->next;
    }
    printf("\n");
}

int contaElementiCoda(Coda * c)
{
    int conta = 0;
    if (c->head == NULL)
        return 0;
    Nodo *temp = c->head;
    while (temp != NULL)
    {
        conta++;
        temp = temp->next;
    }
    return conta;
}


int estraiPunteggio(char *val)
{
    char *virgola = strchr(val, ',');
    if (virgola == NULL)
    {
        return 0; // Valore predefinito se il formato non è corretto
    }
    return atoi(virgola + 2); // +2 per saltare ", " e ottenere il punteggio
}

// Funzione per scambiare i valori di due nodi
void scambiaValori(Nodo *a, Nodo *b)
{
    char *temp = a->val;
    a->val = b->val;
    b->val = temp;
}

// Funzione per ordinare i nodi in base al punteggio
void ordinaCoda(Coda *coda)
{
    if (coda->head == NULL)
    {
        return;
    }

    int scambiato;
    Nodo *ptr1;
    Nodo *lptr = NULL;

    do
    {
        scambiato = 0;
        ptr1 = coda->head;

        while (ptr1->next != lptr)
        {
            if (estraiPunteggio(ptr1->val) < estraiPunteggio(ptr1->next->val))
            {
                scambiaValori(ptr1, ptr1->next);
                scambiato = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (scambiato);
}

char *creaClassifica(Coda *c)
{
    size_t lung = 0;

    // ordiniamo la coda in base al punteggio
    ordinaCoda(c);

    // vediamo quanto deve essere lunga la stringa finale
    Nodo *temp = c->head;
    while (temp != NULL)
    {
        lung = lung + strlen(temp->val) + 1;
        temp = temp->next;
    }

    // creazione del messaggio
    char *csv = (char *)malloc(lung * sizeof(char));

    temp = c->head;

    strcpy(csv, temp->val);
    if(temp->next != NULL)
        strcat(csv, "\n");
    temp = temp->next;

    while (temp != NULL)
    {
        strcat(csv, temp->val);
        if(temp->next != NULL)
            strcat(csv, "\n");
        temp = temp->next;
    }

    free(temp);

    //printf("%s",csv);

    return csv;
}

char *creaBacheca(Coda *c)
{
    size_t lung = 0;

    // vediamo quanto deve essere lunga la stringa finale
    Nodo *temp = c->head;
    while (temp != NULL)
    {
        lung = lung + strlen(temp->val) + 1;
        temp = temp->next;
    }

    // creazione del messaggio
    char *csv = (char *)malloc(lung * sizeof(char));

    temp = c->head;

    strcpy(csv, temp->val);
    strcat(csv, "\n");
    temp = temp->next;

    while (temp != NULL)
    {
        strcat(csv, temp->val);
        if(temp->next != NULL)
            strcat(csv, "\n");
        temp = temp->next;
    }

    free(temp);

    //printf("%s",csv);

    return csv;
}






