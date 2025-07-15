#include "trie.h"
#include <stdlib.h>
#include "code.h"

TrieNode *trie_node_create(uint16_t index) {
	TrieNode *n = malloc(sizeof(TrieNode));
	if (n == NULL) {
		return n;
	}
	n->code = index;
	for (int i = 0; i < ALPHABET; i++) {
		n->children[i] = NULL;
	}
	return n;
}

void trie_node_delete(TrieNode *n) {
	free(n);
}

TrieNode *trie_create(void) {
	return trie_node_create(EMPTY_CODE);
}

void trie_reset(TrieNode *root) {
	for (int i = 0; i < ALPHABET; i++) {
		trie_delete(root->children[i]);
		root->children[i] = NULL;
	}
}

void trie_delete(TrieNode *n) {
	for (int i = 0; i < ALPHABET; i++) {
        if (n->children[i] != NULL) {
			trie_delete(n->children[i]);
        	n->children[i] = NULL;
    	}
	}
	trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
	return n->children[sym];
}

