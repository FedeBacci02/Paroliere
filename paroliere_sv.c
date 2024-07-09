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
#include <sys/select.h>

#include "./librerie/messageStructure.h"
#include "./librerie/matrix.h"
#include "./librerie/macros.h"
#include "./librerie/listaUser.h"
#include "./librerie/listaConcatenataPlayer.h"
#include "./librerie/stringFunction.h"
#include "./librerie/Trie.h"
#include "./librerie/CodaThreads.h"

#define MAX_NUM_CLIENTS 32

// variabili per la sincronizzazione dei thread
pthread_mutex_t mutex_matrix = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tempo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pausa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_durata = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_bacheca = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pausa = PTHREAD_COND_INITIALIZER;
pthread_cond_t scorer = PTHREAD_COND_INITIALIZER;
pthread_cond_t barriera = PTHREAD_COND_INITIALIZER;

// socket e process id
int server_sock, iClient = 0;

// variabili condivise
int seed = -1;     // valore standard
char Matrix[4][4]; // Matrice attuale

volatile sig_atomic_t sigalarm_flag = 0;
volatile sig_atomic_t sigint_flag = 0;
int tempo = 0, tempo_client = 60;
int arrivati = 0;
char *csv = NULL, *csvMessage;

// flags
int PAUSA_SET = 0; // flag che indica se siamo in pausa o meno
int invioPunteggi = 0, classifica = 0;
int check;

// lista giocatori connessi
Lista l = NULL;

// dizionario
char *nomeDizionario = "dictionary_ita.txt"; // dizionario di default
TrieNode *dizionario = NULL;

// struttura dati gestione messaggi
Coda bachecaMessaggi = {NULL, NULL};

// coda utenti registrati non cancellati, verranno estratti solo se l utente lo richiede
Registrati utenti = NULL;

// struttura dati coda per la gestione dei messaggi con il punteggio degli utenti
Coda punteggi = {NULL, NULL};

// inizializzazione del file descriptor del file
FILE *fin;

// handler usati nel progetto
void handle_sigint(int);
void *client_handler(void *);
void *matrix_handler(void *);
void *endGame_handler(void *);
void *scorer_handler(void *);

int main(int argc, char *argv[])
{
  int retvalue;

  // controllo parametri
  if (argc < 3 || argc > 9)
  {
    perror("sintassi errata : inserire ./server nome_server porta_server [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario] [--disconnetti-dopo tempo_in_minuti]");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < argc; i++)
  {
    if (strstr(argv[i], "--disconnetti-dopo"))
    {
      if (isnumeric(argv[i + 1]))
        tempo_client = atoi(argv[i + 1]) * 60;
      else
      {
        perror("sintassi errata : inserire ./server nome_server porta_server [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario] [--disconnetti-dopo tempo_in_minuti]");
        exit(EXIT_FAILURE);
      }
    }

    if (strstr(argv[i], "--seed"))
    {
      if (isnumeric(argv[i + 1]))
      {
        seed = atoi(argv[i + 1]);
        if (seed < 0)
        {
          perror("seed deve essere > 0");
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        perror("sintassi errata : inserire ./server nome_server porta_server [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario] [--disconnetti-dopo tempo_in_minuti]");
        exit(EXIT_FAILURE);
      }
    }

    if (strstr(argv[i], "--diz"))
    {
      nomeDizionario = argv[i + 1];
    }
  }

  // assegnazione della porta e dell'indirizzo ip
  char *ip = argv[1];
  if (strcmp(ip, "localhost") == 0)
    ip = "127.0.0.1";
  if (strcmp(ip, "127.0.0.1") != 0)
  {
    perror("ip non valido");
    exit(EXIT_FAILURE);
  }
  int port = atoi(argv[2]);
  if (port <= 1024)
  {
    perror("porta non valida");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;

  // CREAZIONE DEL SOCKET
  SYSC(server_sock, socket(AF_INET, SOCK_STREAM, 0), "nella server socket");
  printf("[+]TCP server socket created.\n");

  // INIZIALIZZAZIONE SERVER
  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  // BIND
  SYSC(retvalue, bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)), "nella bind");
  printf("[+]Bind to the port number: %d\n", port);

  // LISTEN
  SYSC(retvalue, listen(server_sock, 10), "nella listen");
  printf("[+]Listening...\n");

  // Associazione del gestore per il segnale Ctrl-C
  signal(SIGINT, handle_sigint);

  // creazione del dizionario di controllo
  dizionario = createNode();

  if (!dizionario)
  {
    fprintf(stderr, "Errore nella creazione del nodo root\n");
    return 1;
  }

  // Carica il dizionario dal file
  loadDictionaryFromFile(dizionario, nomeDizionario);

  // apre il file di log in scrittura e modifica per svuotare il file
  fin = fopen("fileDiLog.txt", "w");
  if (!fin)
  {
    perror("Errore nell'apertura del file in modalità scrittura");
    exit(EXIT_FAILURE);
  }
  fclose(fin); // Chiudi il file dopo averlo svuotato

  // Gestore delle partite
  Partita *p = PartitaInit(argc, argv);
  if (p->file != NULL && seed > 0)
  {
    perror("--seed e --matrici non possono essere inserite insieme");
    exit(EXIT_FAILURE);
  }
  pthread_t match;
  if ((retvalue = pthread_create(&match, NULL, matrix_handler, p)) != 0)
  {
    fprintf(stderr, "pthread_create: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }
  if ((retvalue = pthread_detach(match)) != 0)
  {
    fprintf(stderr, "Errore nel detach del thread: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  // ACCEPT
  while (1)
  {
    int client_sock;
    pid_t client_pid;

    do
    {
      // server si mette in ascolto di nuovi client da connettere
      SYSC(client_sock, accept(server_sock, (struct sockaddr *)&client_addr, &addr_size), "nella accept");

      pthread_mutex_lock(&mutex_pausa);

      // se connesso un client prendere il suo PID
      SYSC(retvalue, read(client_sock, &client_pid, sizeof(client_pid)), "nella read");

      // controllare se abbiamo raggiunto il n di client connessi al server
      if (iClient == MAX_NUM_CLIENTS)
      {
        SYSC(retvalue, kill(client_pid, SIGINT), "nella kill");
        pthread_mutex_unlock(&mutex_pausa);
      }
    } while (iClient == MAX_NUM_CLIENTS);

    pthread_t requestClient;
    printf("[+]CLIENT %d CONNESSO AL SERVER \n", client_pid);

    addr_size = sizeof(client_addr);

    addClient(client_pid, client_sock, &l);

    // HANDLER GESTIONE DEL CLIENT CONNESSO
    if ((retvalue = pthread_create(&requestClient, NULL, client_handler, &client_pid)) != 0)
    {
      fprintf(stderr, "pthread_create: %s\n", strerror(retvalue));
      exit(EXIT_FAILURE);
    }

    printf("[+]CLIENT %d E' GESTITO DAL THREAD %ld\n", client_pid, requestClient);
    pthread_detach(requestClient);

    iClient++; // incremento n. client conneessi al gioco
    pthread_mutex_unlock(&mutex_pausa);
  }

  return 0;
}

void *client_handler(void *args)
{
  Player *player = (Player *)malloc(sizeof(Player));
  pid_t *x = (pid_t *)args;
  pid_t pid = *x;

  int retvalue;

  pthread_mutex_lock(&mutex_pausa);
  // printf("Pid: %d", pid);
  player = ricercaNodo(pid, l);

  // printf("%d , %d , %s \n", player->pid_client, player->client_socket, player->val);
  bzero(player->val, BUFFER_SIZE);

  // creazione del thread per gestire la fine della partita
  pthread_t endGame;
  if ((retvalue = pthread_create(&endGame, NULL, endGame_handler, &pid)) != 0)
  {
    fprintf(stderr, "pthread_create: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  };
  if ((retvalue = pthread_detach(endGame)) != 0)
  {
    fprintf(stderr, "Errore nel detach del thread: %d\n", retvalue);
    exit(EXIT_FAILURE);
  }

  pthread_mutex_unlock(&mutex_pausa);

  // printf("%s", player->val);
  int read_size;
  int registrato = 0;
  char buffer[BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);
  Message *msg = initMassage();
  while (1)
  {
    fd_set readfds;
    struct timeval tv;

    // Imposta il timeout
    tv.tv_sec = tempo_client;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(player->client_socket, &readfds);

    int retval = select(player->client_socket + 1, &readfds, NULL, NULL, &tv);

    if (retval == -1)
    {
      perror("select()");
      break;
    }
    else if (retval == 0)
    {
      pthread_mutex_lock(&mutex_pausa);
      // Timeout scaduto, chiudi la connessione
      printf("Timeout scaduto. Il client %d non ha inviato messaggi entro %d secondi.\n", player->pid_client, tempo_client);

      // chiudi il client
      kill(player->pid_client, SIGINT);

      // attesa che il client sia realmente chiuso
      wait_for_process(player->pid_client);

      break;
    }
    else
    {
      if (FD_ISSET(player->client_socket, &readfds))
      {

        read_size = read(player->client_socket, buffer, BUFFER_SIZE);

        if (read_size > 0)
        {

          // Valutazione del messaggio ricevuto
          RiceviMessaggio(buffer, msg);

          // REGISTRAZIONE DELL'UTENTE
          if (msg->type == MSG_REGISTRA_UTENTE)
          {
            pthread_mutex_lock(&mutex_pausa);
            // printf("val: %ld\n", strlen(player->val));
            if (player->val[0] == '\0')
            {
              if (ricercaParola(msg->data, l) == 1 || ricercaUtente(msg->data, utenti) == 1)
              {
                // GESTIONE ERRORE NEL CASO IN CUI IL NOME UTENTE SIA GIA' USATO
                inviaMessaggio(player->client_socket, MSG_ERR, "Nome utente già registrato, cambiare nome!");
              }
              else if (registrato == 1)
              {
                inviaMessaggio(player->client_socket, MSG_ERR, "Sei gia registrato. Puoi iniziare a giocare!");
              }
              else
              {
                pthread_mutex_lock(&mutex_matrix);

                // GESTIONE CONFERMA REGISTRAZIONE
                strcpy(player->val, msg->data);
                printf("[+]%s NUOVO UTENTE REGISTRATO CON SUCCESSO\n", player->val);
                aggiornaFileLog(fin, player->val, "utente registrato nel server");
                // printf("nome attuale: %s", player->val);
                inviaMessaggio(player->client_socket, MSG_OK, "Utente registrato con successo!");
                registrato = 1;
                sleep(1);
                if (PAUSA_SET == 1)
                {
                  // INVIO TEMPO DI ATTESA
                  pthread_mutex_lock(&mutex_tempo);
                  int t = tempo;
                  inviaMessaggio(player->client_socket, MSG_TEMPO_ATTESA, intToString(t));
                  pthread_mutex_unlock(&mutex_tempo);
                }
                else
                {
                  // INVIO MATRICE PARTITA CORRENTE E IL TEMPO RIMASTO
                  inviaMessaggio(player->client_socket, MSG_MATRICE, matrixBuffer(Matrix));
                  sleep(1);
                  pthread_mutex_lock(&mutex_tempo);
                  int t = tempo;
                  inviaMessaggio(player->client_socket, MSG_TEMPO_PARTITA, intToString(t));
                  pthread_mutex_unlock(&mutex_tempo);
                }
                pthread_mutex_unlock(&mutex_matrix);

                printf("[+]GIOCATORI IN PARTITA: ");
                print(l);
                printf("\n");
                pthread_mutex_unlock(&mutex_pausa);
              }
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "Sei già registrato!");
          }

          if (msg->type == MSG_MATRICE)
          {
            if (registrato != 0)
            {
              pthread_mutex_lock(&mutex_pausa);
              if (PAUSA_SET == 0)
              {
                inviaMessaggio(player->client_socket, MSG_MATRICE, matrixBuffer(Matrix));
              }
              else
              {
                pthread_mutex_lock(&mutex_tempo);
                int t = tempo;
                inviaMessaggio(player->client_socket, MSG_TEMPO_ATTESA, intToString(t));
                pthread_mutex_unlock(&mutex_tempo);
              }
              pthread_mutex_unlock(&mutex_pausa);
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "REGISTRATI O LOGGATI PER POTER GIOCARE");
          }

          if (msg->type == MSG_PAROLA)
          {
            pthread_mutex_lock(&mutex_pausa);
            // printf("%ld", strlen(msg->data));
            if (registrato != 0)
            {
              if (PAUSA_SET == 0)
              {
                printf("%s\n", msg->data);
                int punteggio = wordSearch(Matrix, msg->data);
                if (search(dizionario, msg->data) && punteggio > 0 && strlen(msg->data) >= 4)
                {
                  if (searchWord(&(player->paroleInserite), msg->data) != 1)
                  {
                    inviaMessaggio(player->client_socket, MSG_PUNTI_PAROLA, intToString(punteggio));
                    aggiornaFileLog(fin, player->val, msg->data);
                    player->punteggio = player->punteggio + punteggio;
                    push(msg->data, &(player->paroleInserite));
                    printf("punteggio ottenuto: %d\n", punteggio);
                  }
                  else
                  {
                    inviaMessaggio(player->client_socket, MSG_PUNTI_PAROLA, "0");
                  }
                }
                else
                {
                  if (strlen(msg->data) < 4)
                    inviaMessaggio(player->client_socket, MSG_ERR, "LA PAROLA NON DEVE AVERE MENO DI 4 CARATTERI!");
                  else
                    inviaMessaggio(player->client_socket, MSG_ERR, "PAROLA INSERITA NON VALIDA!");
                }
              }
              else
                inviaMessaggio(player->client_socket, MSG_ERR, "IL GIOCO E' IN PAUSA");
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "REGISTRATI O LOGGATI PER POTER GIOCARE");
            pthread_mutex_unlock(&mutex_pausa);
          }

          if (msg->type == MSG_CANCELLA_UTENTE)
          {
            pthread_mutex_lock(&mutex_pausa);
            if (registrato == 1)
            {
              // cancellazione account
              inviaMessaggio(player->client_socket, MSG_OK, "Cancellazione effettuata con successo!");
              aggiornaFileLog(fin, player->val, "utente cancellato con successo");
              registrato = 0;
              printf("[+] %s: UTENTE CANCELLATO\n", player->val);
              memset(player->val, 0, BUFFER_SIZE);
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "non sei ancora registra o loggato!");
            pthread_mutex_unlock(&mutex_pausa);
          }
          // GESTIONE DEL MESSAGGIO MSG_LOGIN_UTENTE
          if (msg->type == MSG_LOGIN_UTENTE)
          {
            pthread_mutex_lock(&mutex_pausa);
            if (registrato == 1)
              inviaMessaggio(player->client_socket, MSG_ERR, "sei gia loggato, puoi iniziare a giocare!");
            else
            {
              if (ricercaUtente(msg->data, utenti) == 1)
              {
                strcpy(player->val, msg->data);
                inviaMessaggio(player->client_socket, MSG_OK, "Utente loggato con successo!");
                cancellaUtente(player->val, &utenti);
                registrato = 1;
                printf("[+] %s: UTENTE LOGGATO\n", player->val);
              }
              else
                inviaMessaggio(player->client_socket, MSG_ERR, "utente non registrato nel sistema");
            }
            pthread_mutex_unlock(&mutex_pausa);
          }

          // GESTIONE DEL MESSAGGIO POST BACHECA
          if (msg->type == MSG_POST_BACHECA)
          {
            pthread_mutex_lock(&mutex_bacheca);
            if (registrato == 1)
            {
              if (strlen(msg->data) <= 128)
              {
                char messaggio[2046];
                sprintf(messaggio, "%s, %s", player->val, msg->data);
                if (contaElementiCoda(&bachecaMessaggi) == 8)
                  pop(&bachecaMessaggi);
                push(messaggio, &bachecaMessaggi);
                inviaMessaggio(player->client_socket, MSG_OK, "messaggio inserito correttamente!");
              }
              else
                inviaMessaggio(player->client_socket, MSG_ERR, "messaggio troppo lungo!");
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "REGISTRATI O LOGGATI PER POTER ACCEDERE ALLA BACHECA");
            pthread_mutex_unlock(&mutex_bacheca);
          }

          if (msg->type == MSG_SHOW_BACHECA)
          {
            pthread_mutex_lock(&mutex_bacheca);
            if (registrato == 1)
            {
              if (bachecaMessaggi.head != NULL)
              {
                // printCoda(&bachecaMessaggi);
                csvMessage = creaBacheca(&bachecaMessaggi);
                inviaMessaggio(player->client_socket, MSG_SHOW_BACHECA, csvMessage);
                free(csvMessage);
              }
              else
                inviaMessaggio(player->client_socket, MSG_ERR, "bacheca messaggi vuota");
            }
            else
              inviaMessaggio(player->client_socket, MSG_ERR, "REGISTRATI O LOGGATI PER POTER ACCEDERE ALLA BACHECA");
            pthread_mutex_unlock(&mutex_bacheca);
          }

          // svuotiamo il buffer
          bzero(buffer, BUFFER_SIZE);
        }
        // Controlla se il client si è disconnesso o se si è verificato un errore
        else if (read_size == 0)
        {
          pthread_mutex_lock(&mutex_pausa);
          printf("[+] IL CLIENT %d SI E' DISCONNESSO\n", player->pid_client);
          break;
        }
        else if (read_size == -1)
        {
          pthread_mutex_lock(&mutex_pausa);
          perror("read ha fallito");
          break;
        }
      }
    }
  }

  if (sigint_flag != 1)
  {
    // Rimuovi l'utente dalla partita e libera le risorse allocate

    // salva l utente registrato non cancellato in una struttura dati dei utenti registrati
    if (registrato == 1)
      registraUtente(player->val, &utenti);

    deleted(player->pid_client, &l);
    printf("Giocatori in partita:\n");
    print(l);
    printf("\n");

    printf("[+] users registrati: ");
    printUsers(utenti);

    iClient--;
  }
  pthread_mutex_unlock(&mutex_pausa);

  // assicurarsi che player sia vuoto
  player = NULL;

  // Libera la memoria allocata per il messaggio
  free(msg);

  // Termina il thread
  pthread_exit(NULL);
}

// GESTORE DEL SEGNALE SIG INT
void handle_sigint(int sig)
{
  int retvalue;

  pthread_mutex_lock(&mutex_pausa);
  // informiamo i thread che stiamo chiudendo il server
  sigint_flag = 1;

  pthread_mutex_unlock(&mutex_pausa);
  // output
  const char msg[] = "\n Ricevuto segnale ctrl-C \n Arresto del server.\n";
  write(STDOUT_FILENO, msg, sizeof(msg) - 1);

  // chiusura dei client ancora connessi al server
  destroy(&l);

  // chiusura del socket del server
  SYSC(retvalue,close(server_sock),"nella close");

  // chiusura del programma server
  exit(EXIT_SUCCESS);
}

// GESTORE DEL TEMPO E DELLE PARTITE
void *matrix_handler(void *args)
{
  Partita *p = (Partita *)args;
  int retvalue;

  // thread gestione della classifica
  pthread_t scorer;
  if ((retvalue = pthread_create(&scorer, NULL, scorer_handler, NULL) != 0))
  {
    fprintf(stderr, "pthread_create: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  if ((retvalue = pthread_detach(scorer)) != 0)
  {
    fprintf(stderr, "Errore nel detach del thread: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    // inizio game
    // printf("classifica: %d \n", classifica);
    // printf("arrivati: %d \n", arrivati);
    // printf("punteggimandagi: %d \n", invioPunteggi);

    // generazione della matrice della partita

    pthread_mutex_lock(&mutex_pausa);
    pthread_mutex_lock(&mutex_matrix);
    generaMatrice(Matrix, p, seed);
    pthread_mutex_unlock(&mutex_matrix);

    // azzero la classifica
    free(csv);
    csv = NULL;

    clearQueue(&punteggi);

    tempo = p->durata;

    PAUSA_SET = 0;
    printf("[+]PARTITA INIZIATA\n");
    pthread_mutex_unlock(&mutex_pausa);

    while (tempo != 0)
    {
      sleep(1);
      pthread_mutex_lock(&mutex_tempo);
      tempo--;
      pthread_mutex_unlock(&mutex_tempo);
      // printf("time: %d \n", tempo);
    }

    pthread_mutex_lock(&mutex_pausa);
    tempo = 60;

    // attiviamo la creazione della classifica
    PAUSA_SET = 1;
    invioPunteggi = 1;
    pthread_cond_broadcast(&pausa);

    while (invioPunteggi == 1 && iClient != 0)
      pthread_cond_wait(&pausa, &mutex_pausa);

    // printf("invioPunteggi: %d\n", invioPunteggi);
    printf("[+]PARTITA FINITA - INIZIO PAUSA\n");
    pthread_mutex_unlock(&mutex_pausa);

    while (tempo != 0)
    {
      sleep(1);
      pthread_mutex_lock(&mutex_tempo);
      tempo--;
      pthread_mutex_unlock(&mutex_tempo);
      // printf("time: %d \n", tempo);
    }
  }

  // chiusura thread
  free(p);
  return NULL;
}

void *endGame_handler(void *args)
{
  Player *player = (Player *)malloc(sizeof(Player));
  pid_t *x = (pid_t *)args;
  pid_t pid = *x;

  pthread_mutex_lock(&mutex_lista);
  pthread_mutex_unlock(&mutex_lista);

  char data[2048];

  while (1)
  {
    pthread_mutex_lock(&mutex_pausa);
    while (invioPunteggi == 0)
      pthread_cond_wait(&pausa, &mutex_pausa);

    player = ricercaNodo(pid, l);

    if (player == NULL)
      break;

    if (player->val[0] != '\0')
    {
      sprintf(data, "%s, %d", player->val, player->punteggio);
      push(data, &punteggi);
      sprintf(data, "%d punti ottenuti", player->punteggio);
      aggiornaFileLog(fin, player->val, data);
    }

    arrivati++;
    if (arrivati == iClient)
    {
      // svegliamo lo scorer
      classifica = 1;
      // printCoda(&punteggi);
      // fflush(stdout);
      printf("[+]SCORER ATTIVATO\n");
      pthread_cond_signal(&scorer);
    }

    while (invioPunteggi == 1)
      pthread_cond_wait(&barriera, &mutex_pausa);

    if (player->val[0] != '\0')
      inviaMessaggio(player->client_socket, MSG_PUNTI_FINALI, csv);

    player->punteggio = 0;
    clearQueue(&(player->paroleInserite));

    pthread_mutex_unlock(&mutex_pausa);
  }

  pthread_mutex_unlock(&mutex_pausa);

  // nel caso il thread che gestisce il client venga chiuso
  return NULL;
}

void *scorer_handler(void *args)
{
  while (1)
  {
    pthread_mutex_lock(&mutex_pausa);
    while (classifica == 0)
      pthread_cond_wait(&scorer, &mutex_pausa);

    if (punteggi.head != NULL)
    {
      // funzione che ordina la coda
      ordinaCoda(&punteggi);

      // creazione del messaggio
      csv = creaClassifica(&punteggi);
      printf("[+]classifica fatta, vincotore nominato!\n");

      // printf("%s", csv);
      // printf("\n");
    }

    classifica = 0;
    arrivati = 0;
    invioPunteggi = 0;
    sleep(1);
    pthread_cond_broadcast(&barriera);
    pthread_cond_signal(&pausa);
    pthread_mutex_unlock(&mutex_pausa);
  }
}