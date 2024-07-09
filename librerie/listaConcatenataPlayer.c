#include "listaConcatenataPlayer.h"
#include "CodaThreads.h"

/* Inserisce, ricorsivamente, un elemento in lista*/
Player *addClient(pid_t pid, int client_sock, Lista *l)
{
    if (*l == NULL)
    {
        Player *nuovo = (Player *)malloc(sizeof(Player));
        if (nuovo == NULL)
        {
            perror("Errore nell'allocazione della memoria");
            exit(1);
        }
        nuovo->pid_client = pid;
        nuovo->client_socket = client_sock;
        nuovo->val[0] = '\0'; // Inizializza `val` come stringa vuota
        nuovo->punteggio = 0; 
        CodaInit(&(nuovo->paroleInserite));
        nuovo->next = *l;
        *l = nuovo;
        return nuovo;
    }
    else
    {
        return addClient(pid, client_sock, &((*l)->next));
    }
}

/* stampa tutti gli elementi di una lista, partendo dalla testa, ricorsivamente */
void print(Lista l)
{
    if (l == NULL)
        return;
    printf("%s -> ", l->val);
    print(l->next);
}

// ricerca valore nella lista, ritorna 1 se trovata, 0 altrimenti
int ricercaParola(char x[], Lista l)
{
    if (l == NULL)
        return 0;
    if (strcmp(x, l->val) == 0)
        return 1;
    return ricercaParola(x, l->next);
}

Player *ricercaNodo(pid_t x, Lista l)
{
    if (l == NULL)
        return NULL;
    if (x == l->pid_client)
        return l;
    return ricercaNodo(x, l->next);
}

void deleted(pid_t x, Lista *L)
{
    Player *attuale = *L;
    Player *prec = NULL;

    while (attuale != NULL)
    {

        if (attuale->pid_client == x)
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
            memset(attuale->val, 0, BUFFER_SIZE);
            close(attuale->client_socket);
            free(attuale);
            return;
        }
        else
        {
            prec = attuale;
            attuale = attuale->next;
        }
    }
}

int is_process_running(pid_t pid) {
    // kill(pid, 0) non invia nessun segnale, ma controlla se il processo è esistente.
    if (kill(pid, 0) == 0) {
        return 1; // Processo ancora in esecuzione
    } else {
        return 0; // Processo terminato o non esistente
    }
}

void wait_for_process(pid_t pid) {
    while (is_process_running(pid)) {
        sleep(1); // Aspetta 1 secondo prima di ricontrollare
    }
    //printf("[+] IL CLIENT %d E' TERMINATO\n", pid);
}

/* Distrugge la lista liberando tutta la memoria allocata e chiudendo i socket */
void destroy(Lista *L)
{
    Player *attuale = *L;
    while (attuale != NULL)
    {
        Player *temp = attuale;
        attuale = attuale->next;

        // chiudi il client
        kill(temp->pid_client, SIGINT);

        //attesa che il client sia realmente chiuso
        wait_for_process(temp->pid_client);

        // Chiudi il socket del client
        close(temp->client_socket);

        // Libera la memoria del nodo
        free(temp);
    }
    *L = NULL;
}


