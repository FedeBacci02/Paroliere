#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "messageStructure.h"

/* inizializza il messaggio */
Message *initMassage()
{
    // creazione del messaggio
    Message *msg = (Message *)malloc(sizeof(Message));
    

    msg->data = NULL;
    msg->lenght = 0;

    return msg;
}

/* viene spacchettato il messaggio e viene assegnato ad msg l'msg->type e msg->data se esiste */

void RiceviMessaggio(char buffer[], Message *msg)
{   
    //fprintf(stderr,"RiceviMessaggio: %s\n",buffer);
    //int lenght = 0;
    msg->type=buffer[0];
    if(buffer[1]!='\0'){
        memmove(buffer, buffer+1, strlen(buffer));
        msg->data = buffer;
        //lenght = strlen(msg->data);
    }
    //fprintf(stderr,"RiceviMessaggio: %s\n",buffer);
}

/* impacchettamento del messaggio e la spedizione */

void inviaMessaggio(int client_sock, char type, char *data) {       
    int lenght = 0;
    //printf("%c\n", type);
    if (data != NULL) {
        lenght = strlen(data);
        //printf("%d \n", lenght);
    }

    char *newString = (char *)malloc(1 + lenght + 1); // 1 per il tipo, lenght per i dati, e 1 per '\0'
    if (newString == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE); // Gestione dell'errore di allocazione
    }

    newString[0] = type;
    if (data != NULL) {
        strncpy(newString + 1, data, lenght);
    }
    newString[1 + lenght] = '\0'; // Aggiungere il terminatore nullo

    //printf("%s\n", newString);

    write(client_sock, newString, 1 + lenght);

    free(newString);
}