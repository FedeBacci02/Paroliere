#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int words(const char sentence[])
{
    int counted = 0; // result

    // state:
    const char *it = sentence;
    int inword = 0;

    do
        switch (*it)
        {
        case '\0':
        case ' ':
        case '\t':
        case '\n':
        case '\r': // TODO others?
            if (inword)
            {
                inword = 0;
                counted++;
            }
            break;
        default:
            inword = 1;
        }
    while (*it++);

    return counted;
}

const char *cmdString(char *string, int index)
{
    char *parametro;
    char *tok;
    char *newString = strdup(string);

    int i = 0;
    tok = strtok(newString, " ");
    if (i == index)
        parametro = tok;
    i++;
    while ((tok = strtok(NULL, " ")) != NULL)
    {
        if (i == index)
        {
            parametro = tok;
        }

        i++;
    }
    return parametro;
}

char *intToString(int numero)
{
    int cifre = 0;
    int temp = numero; // Salva il valore originale di numero
    while (temp != 0)
    {
        temp /= 10;
        cifre++;
    }

    char *stringa = (char *)malloc(cifre + 1); // Alloca memoria per la stringa

    // Converte il numero in una stringa
    snprintf(stringa, cifre + 1, "%d", numero);

    return stringa;
}

int isnumeric(char *str)
{
    while (*str)
    {
        if (!isdigit(*str))
            return 0;
        str++;
    }

    return 1;
}
void aggiornaFileLog(FILE * file,char * utente, char *string)
{
    file = fopen("fileDiLog.txt", "a");
    if (file == NULL)
    {
        perror("Errore nell'apertura del file in modalità append");
        exit(EXIT_FAILURE);
    }

    // Aggiungi dati al file
    fprintf(file, "%s -> %s; \n",utente,string);

    // Chiudi il file
    fclose(file);
}
