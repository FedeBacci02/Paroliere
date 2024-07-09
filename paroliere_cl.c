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
#include <signal.h>
#include <errno.h>

#include "./librerie/messageStructure.h"
#include "./librerie/stringFunction.h"
#include "./librerie/macros.h"
#include "./librerie/matrix.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request = PTHREAD_COND_INITIALIZER;
int ricevuto = 0;

int sock;

// flag
int registrato = 0; // se reg=1 altrimenti 0

void handle_sigint(int sig)
{
  int retvalue;
  const char msg[] = "\n Segnale di chiusura ricevuto. \n Arresto del client.\n";
  write(STDOUT_FILENO, msg, sizeof(msg) - 1);
  SYSC(retvalue, close(sock), "nella close");
  exit(EXIT_SUCCESS);
}

void *handler_richieste(void *args)
{
  char cmd[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];
  char nomeUtente[BUFFER_SIZE];

  bzero(nomeUtente, BUFFER_SIZE);
  bzero(buffer, BUFFER_SIZE);
  bzero(cmd, BUFFER_SIZE);

  Message *msg = initMassage();

  ricevuto = 0;

  while (1)
  {
    pthread_mutex_lock(&mutex);
    while (ricevuto != 0)
    {
      //printf("sono qui\n");
      pthread_cond_wait(&request, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    printf("\033[1m[PROMPT PAROLIERE]-->\033[m ");
    fgets(cmd, sizeof(cmd), stdin);

    pthread_mutex_lock(&mutex);
    cmd[strlen(cmd)] = '\0';

    if (strstr(cmd, "p "))
    {
      char parola[BUFFER_SIZE];
      if (words(cmd) == 2)
      {
        strcpy(parola, cmdString(cmd, 1));

        parola[strcspn(parola, "\n")] = '\0'; // Rimuove il newline
        inviaMessaggio(sock, MSG_PAROLA, parola);

        // risposte attese
        ricevuto = 1;
      }
      else
        printf("la parola inserita deve essere senza spazi\n");
    }

    else if (strstr(cmd, "matrice"))
    {
      // richiesta della matrice e del tempo residuo
      inviaMessaggio(sock, MSG_MATRICE, NULL);

      // risposte attese
      ricevuto = 1;
    }
    else if (strstr(cmd, "registra utente"))
    {
      // REGISTRAZIONE DELL'UTENTE
      if (words(cmd) == 3)
      {
        // Estrazione del nome utente e rimozione del newline finale
        strcpy(nomeUtente, cmdString(cmd, 2));
        nomeUtente[strcspn(nomeUtente, "\n")] = '\0'; // Rimuove il newline

        // INVIO MESSAGGIO
        inviaMessaggio(sock, MSG_REGISTRA_UTENTE, nomeUtente);

        // risposte attese
        ricevuto = 3;
      }
      else
      {
        printf("Parametri errati; inserire 'registra utente [Nome utente]'\n");
      }
    }
    else if (strstr(cmd, "login_utente"))
    {
      // REGISTRAZIONE DELL'UTENTE
      if (words(cmd) == 2)
      {
        // Estrazione del nome utente e rimozione del newline finale
        bzero(nomeUtente, BUFFER_SIZE);
        strcpy(nomeUtente, cmdString(cmd, 1));
        nomeUtente[strcspn(nomeUtente, "\n")] = '\0'; // Rimuove il newline

        // INVIO MESSAGGIO
        inviaMessaggio(sock, MSG_LOGIN_UTENTE, nomeUtente);

        // risposte attese
        ricevuto = 1;
      }
      else
      {
        printf("Parametri errati; inserire 'registra utente [Nome utente]'\n");
      }
    }
    else if (strstr(cmd, "cancella_utente"))
    {
      if (words(cmd) == 1)
      {
        // INVIO MESSAGGIO
        inviaMessaggio(sock, MSG_CANCELLA_UTENTE, NULL);

        // risposte attese
        ricevuto = 1;
      }
      else
      {
        printf("Parametri errati; inserire 'registra utente [Nome utente]'\n");
      }
    }
    else if (strstr(cmd, "fine"))
    {
      // DISCONESSIONE DEL CLIENT
      free(msg);
      printf("Disconnected from the server.\n");
      pthread_mutex_unlock(&mutex);

      return NULL;
    }
    else if (strstr(cmd, "show_msg"))
    {
      if (words(cmd) == 1)
      {
        // invio del msg al server
        inviaMessaggio(sock, MSG_SHOW_BACHECA, buffer);

        // risposte ricevute
        ricevuto = 1;
      }
      else
      {
        printf("Parametri errati; inserire 'registra utente [Nome utente]'\n");
      }
    }
    else if (strstr(cmd, "msg"))
    {
      int parole = words(cmd);
      if (parole > 1)
      {
        // invio del msg al server
        int strings = 1;
        //printf("%s", cmd);
        bzero(buffer, BUFFER_SIZE);
        strcpy(buffer, cmdString(cmd, strings));
        strcat(buffer, " ");
        strings++;

        while (parole > strings)
        {
          strcat(buffer, cmdString(cmd, strings));
          strings++;
          if (strings != parole)
            strcat(buffer, " ");
          // printf("buffer: %s",buffer);
        }

        buffer[strcspn(buffer, "\n")] = '\0'; // Rimuove il newline

        inviaMessaggio(sock, MSG_POST_BACHECA, buffer);

        // risposte ricevute
        ricevuto = 1;
      }
      else
      {
        printf("Parametri errati; inserire 'registra utente [Nome utente]'\n");
      }
    }
    else if (strstr(cmd, "aiuto"))
    {
      // elenco dei comandi
      printf("registra utente [nome utente] -> comando utilizzato per registrare un nuovo utente\n");
      printf("matrice -> comando utilizzato per richiedere la matrice corrente\n");
      printf("p [parole indicata] -> comando utilizzato per sottoporre al gioco una parola\n");
      printf("login_utente [nome utente] -> comando per effetturare il login\n");
      printf("cancella_utente -> comando utilizzato per cancellare l'utente registrato\n");
      printf("msg [messaggio] -> comando utilizzato inserire un messaggio nella bacheca messaggi\n");
      printf("show_msg -> comando utilizzato per visualizzare la bacheca dei messaggi\n");
      printf("fine -> comando utilizzato per uscire dal gioco.\n");
    }
    else
    {
      // default
      printf("\033[0;31mComando inesistente; inserire 'aiuto' per avere la lista dei comandi\033[0;37m\n");
    }
    pthread_mutex_unlock(&mutex);
  }
}

// GESTISCE TUTTI I MESSAGGI INVIATI DAL SERVER
void *handler_risposte(void *args)
{

  // variabili di supporto
  char buffer[BUFFER_SIZE];
  ssize_t read_size;
  Message *msg = initMassage();

  // ricezione del messaggio dal server
  while ((read_size = read(sock, buffer, BUFFER_SIZE)) > 0)
  {
    pthread_mutex_lock(&mutex);
    // azzeramento del buffer
    RiceviMessaggio(buffer, msg);
    // messaggio di errore
    if (msg->type == MSG_ERR)
    {
      printf("%s\n", msg->data);
      ricevuto = 0;
    }
    // messaggio di ok
    else if (msg->type == MSG_OK)
    {
      printf("%s\n", msg->data);
      ricevuto--;
    }

    // messaggio invio matrice
    else if (msg->type == MSG_MATRICE)
    {
      deBufferMatrix(msg->data);
      ricevuto--;
    }

    // messaggio invio tempo partita rimanente
    else if (msg->type == MSG_TEMPO_PARTITA)
    {
      printf("TEMPO RESTANTE: %s \n", msg->data);
      ricevuto--;
    }

    else if (msg->type == MSG_TEMPO_ATTESA)
    {
      printf("TEMPO DI ATTESA: %s \n", msg->data);
      ricevuto = 0;
    }

    else if (msg->type == MSG_PUNTI_PAROLA)
    {
      printf("Parola esatta!. Punteggio ottenuto: %s\n", msg->data);
      ricevuto--;
    }

    else if (msg->type == MSG_PUNTI_FINALI)
    {
      printf("\nCLASSIFICA GIOCO:\n%s", msg->data);
      if (ricevuto == 0)
      {
        printf("\n\033[1m[PROMPT PAROLIERE]-->\033[m ");
        fflush(stdout);
      }

      ricevuto = 0;
    }

    else if (msg->type == MSG_SHOW_BACHECA)
    {
      printf("BACHECA MESSAGGI: \n%s\n", msg->data);
      ricevuto = 0;
    }

    // printf("%d",ricevuto);

    if (ricevuto == 0)
    {
      pthread_cond_signal(&request);
      // printf("\n\033[1m[PROMPT PAROLIERE]-->\033[m ");
    }

    bzero(buffer, BUFFER_SIZE);
    pthread_mutex_unlock(&mutex);
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    perror("sintassi errata : ./paroliere_cl nome_server porta_server");
    exit(EXIT_FAILURE);
  }

  // PORTA E IND. IP
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

  // VARIABILI DI INIT E SUPPORT
  int retvalue;
  struct sockaddr_in addr;

  // CREAZIONE
  SYSC(sock, socket(AF_INET, SOCK_STREAM, 0), "nella socket");

  printf("[+]TCP server socket created.\n");

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = inet_addr(ip);

  // CONNESSIONE AL SERVER
  SYSC(retvalue, connect(sock, (struct sockaddr *)&addr, sizeof(addr)), "nella connect");
  printf("Connected to the server.\n");

  // GESTIONE DEL CLIENT SULLE RICHIESTE AL SERVER
  pthread_t richiesteClient;
  if ((retvalue = pthread_create(&richiesteClient, NULL, handler_richieste, &sock)) != 0)
  {
    fprintf(stderr, "pthread_join: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  // GESTIONE DEL CLIENT SULLE RICHIESTE MANDATE DAL SERVER
  pthread_t rispostaClient;
  if ((retvalue = pthread_create(&rispostaClient, NULL, handler_risposte, &sock)) != 0)
  {
    fprintf(stderr, "pthread_create: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  if ((retvalue = pthread_detach(rispostaClient)) != 0)
  {
    fprintf(stderr, "pthread_detach: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  // Invia il PID al server
  pid_t pid = getpid();

  if (write(sock, &pid, sizeof(pid)) <= 0)
  {
    perror("nella write");
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, handle_sigint);
  // in attesa che il client finisca
  if ((retvalue = pthread_join(richiesteClient, NULL)) != 0)
  {
    fprintf(stderr, "pthread_join: %s\n", strerror(retvalue));
    exit(EXIT_FAILURE);
  }

  // Chiude il socket
  SYSC(retvalue, close(sock), "nella close");

  // chiusura del client
  exit(EXIT_SUCCESS);
}