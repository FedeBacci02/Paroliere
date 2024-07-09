#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Trie.h"

// Funzione per creare un nuovo nodo del Trie
TrieNode* createNode(void) {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        node->isEndOfWord = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
    }
    return node;
}

// Funzione per inserire una parola nel Trie
void insert(TrieNode *root, const char *key) {
    TrieNode *currentNode = root;
    while (*key) {
        if (isalpha(*key)) {  // Assicurarsi che il carattere sia alfabetico
            int index = tolower(*key) - 'a';
            if (index < 0 || index >= ALPHABET_SIZE) {
                key++;
                continue;
            }
            if (!currentNode->children[index]) {
                currentNode->children[index] = createNode();
            }
            currentNode = currentNode->children[index];
        }
        key++;
    }
    currentNode->isEndOfWord = 1;
}

// Funzione per cercare una parola nel Trie
int search(TrieNode *root, const char *key) {
    TrieNode *currentNode = root;
    while (*key) {
        if (isalpha(*key)) {  // Assicurarsi che il carattere sia alfabetico
            int index = tolower(*key) - 'a';
            if (index < 0 || index >= ALPHABET_SIZE || !currentNode->children[index]) {
                return 0;
            }
            currentNode = currentNode->children[index];
        }
        key++;
    }
    return (currentNode != NULL && currentNode->isEndOfWord);
}

// Funzione per deallocare il Trie
void freeTrie(TrieNode *root) {
    if (!root) return;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        freeTrie(root->children[i]);
    }
    free(root);
}

// Funzione per leggere le parole da un file e inserirle nel Trie
void loadDictionaryFromFile(TrieNode *root, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Impossibile aprire il file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char word[MAX_WORD_LENGTH];
    while (fgets(word, sizeof(word), file)) {
        // Rimuovi il carattere di nuova linea, se presente
        char *newline = strchr(word, '\n');
        if (newline) {
            *newline = '\0';
        }
        insert(root, word);
    }

    fclose(file);
}