#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_CANCELLA_UTENTE 'D'
#define MSG_LOGIN_UTENTE 'L'
#define MSG_POST_BACHECA 'H'
#define MSG_SHOW_BACHECA 'S'

#define BUFFER_SIZE 1024

typedef struct
{
    char type;           // contiene il tipo del messaggio spedito
    unsigned int lenght; // indica il numero dei dati significativi all’interno del campo data
    char *data;          // contiene il messaggio di risposta effettiva
} Message;

/* inizializza il messaggio */

Message *initMassage();
void RiceviMessaggio(char buffer[], Message *msg);
void inviaMessaggio(int client_sock, char type, char *data);

#endif