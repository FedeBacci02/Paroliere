#ifndef MATRIX_H
#define MATRIX_H

#define BUFFER_SIZE 1024
#define ROWS 4
#define COLS 4

typedef struct
{
    char *file;
    int durata;
    int linea;
} Partita;

Partita *PartitaInit(int argc, char *argv[]);
void MatrixRandom(char Matrix[4][4], int seed);
char *chooseLineRandom(int *riga, const char *fileName);
void generateMatrixFile(Partita *p, const char *FileName, char Matrix[4][4]);
void MatrixOutput(char Matrix[4][4]);
void generaMatrice(char Matrix[4][4], Partita *p, int seed);
char *matrixBuffer(char Matrix[4][4]);
void deBufferMatrix(char *buffer);
int wordCheck(char matrix[4][4], char *word, int i, int r, int c);
int conta_carattere(const char *stringa, char carattere);
int wordSearch(char matrix[4][4], char *word);

#endif