#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "matrix.h"



Partita *PartitaInit(int argc, char *argv[])
{
    Partita *p = (Partita *)malloc(sizeof(Partita));
    p->file = NULL;  // no file
    p->durata = 180; // durata di default
    p->linea = 0;    // linea del file da leggere

    for (int i = 0; i < argc; i++)
    {
        if (strstr(argv[i], "--matrici"))
        {
            p->file = argv[i + 1];
        }

        if (strstr(argv[i], "--durata"))
        {
            p->durata = atoi(argv[i + 1]);
        }
    }

    return p;
}

void MatrixRandom(char Matrix[4][4], int seed)
{
    if (seed != -1)
        srand(seed);
    else
        srand(time(NULL));
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVXYZ";
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int x = rand() % 24;
            // printf("x= %d ",x);
            Matrix[i][j] = letters[x];
            // printf("%s ", Matrix[i][j]);
        }
        // printf("\n");
    }

    return;
}

char *chooseLineRandom(int *riga, const char *fileName)
{
    printf("[+] FILE MATRICE USATO: %s\n", fileName);
    FILE *fp;
    char *line = NULL;
    char x[BUFFER_SIZE];
    int contaLinee = 0, lineaScelta;
    size_t len = 0;
    ssize_t read;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
    // printf("\n");
    lineaScelta = *riga;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (lineaScelta == contaLinee)
        {
            strcpy(x, line);
            (*riga)++;
            // printf("%d\n",*riga);
        }
        contaLinee++;
    }
    if (*riga == contaLinee)
        *riga = 0;

    // printf("%s",x);

    fclose(fp);
    if (line)
        free(line);
    // printf("%s", x);
    return strdup(x);
}

void generateMatrixFile(Partita *p, const char *FileName, char Matrix[4][4])
{
    char *matrice2 = chooseLineRandom(&(p->linea), FileName);
    // printf("%s %ld",matrice2,strlen(matrice2));

    int index = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Matrix[i][j] = matrice2[index];
            if (matrice2[index] == 'Q')
            {
                index = index + 3;
            }
            else
                index = index + 2;
        }
    }

    return;
}

void MatrixOutput(char Matrix[4][4])
{
    printf("----------- \n");
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (Matrix[i][j] == 'Q')
                printf("Qu ");
            else
                printf("%c  ", Matrix[i][j]);
        }
        printf("\n");
    }
    printf("----------- \n");
}

void generaMatrice(char Matrix[4][4], Partita *p, int seed)
{

    if (p->file != NULL)
    {
        // generiamo la matrice in base al file
        generateMatrixFile(p, p->file, Matrix);
    }
    else
        // generiamo la matrice casualmente
        MatrixRandom(Matrix, seed);

    printf("[+]MATRICE: \n");
    MatrixOutput(Matrix);
}

char *matrixBuffer(char Matrix[4][4])
{
    char *buffer = (char *)malloc((4 * 4 + 1) * sizeof(char)); // Alloca memoria per contenere tutte le stringhe in matrice

    int k = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            buffer[k++] = Matrix[i][j];
        }
    }
    buffer[k] = '\0'; // Aggiungere il terminatore nullo

    // Utilizzare il buffer (ad esempio, stamparlo)
    // printf("Buffer: %s\n", buffer);

    return buffer;
}

void deBufferMatrix(char *buffer)
{

    int buffer_length = strlen(buffer);
    // printf("%d",buffer_length);

    // Verificare che il buffer abbia la lunghezza corretta
    if (buffer_length != 4 * 4)
    {
        perror("Errore: la lunghezza del buffer non corrisponde alla dimensione della matrice.\n");
        exit(EXIT_FAILURE);
    }

    // Allocare la matrice
    char matrix[4][4];

    // Copiare i caratteri dal buffer nella matrice
    int k = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            matrix[i][j] = buffer[k++];
        }
    }

    printf("Matrice: \n");
    MatrixOutput(matrix);
}

// cerca la parola nella matrice
int wordCheck(char matrix[4][4], char *word, int i, int r, int c)
{
    int indice = i;
    // printf("%d",i);
    if (word[i] == '\0') // Se abbiamo trovato tutti i caratteri della parola
        return 1;

    if (r < 0 || r >= 4 || c < 0 || c >= 4) // Controllo limiti matrice
        return 0;
    // printf("%c == %c\n",word[i],matrix[r][c]);
    if (matrix[r][c] == toupper(word[i]))
    {
        // controllo carattere Qu
        if (toupper(word[i]) == 'Q')
        {
            if (toupper(word[i + 1]) == 'U')
            {
                // printf("%c\n",word[i+1]);
                indice = indice + 2;
            }
            else
                return 0;
        }
        else
            indice++;

        char temp = matrix[r][c];
        matrix[r][c] = '*'; // Marca il carattere come visitato

        // Cerca nelle quattro direzioni
        int found = wordCheck(matrix, word, indice, r + 1, c) ||
                    wordCheck(matrix, word, indice, r - 1, c) ||
                    wordCheck(matrix, word, indice, r, c + 1) ||
                    wordCheck(matrix, word, indice, r, c - 1);

        matrix[r][c] = temp; // Ripristina il carattere originale

        return found;
    }
    else
    {
        return 0;
    }
}

int conta_carattere(const char *stringa, char carattere)
{
    int contatore = 0;

    // Scorri ogni carattere nella stringa
    while (*stringa)
    {
        if (*stringa == carattere)
        {
            contatore++;
        }
        stringa++;
    }

    return contatore;
}

// funzione che conta il punteggio in caso la parola sia ritrovata nella matrice

int wordSearch(char matrix[4][4], char *word)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (wordCheck(matrix, word, 0, i, j))
                return strlen(word) - conta_carattere(word, 'q');
        }
    }
    return 0;
}