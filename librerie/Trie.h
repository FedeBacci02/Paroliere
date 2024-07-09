#ifndef TRIE_H
#define TRIE_H


#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100

// Struttura del nodo del Trie
typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    int isEndOfWord;
} TrieNode;

TrieNode* createNode(void);
void insert(TrieNode *root, const char *key);
int search(TrieNode *root, const char *key);
void freeTrie(TrieNode *root);
void loadDictionaryFromFile(TrieNode *root, const char *filename);



#endif