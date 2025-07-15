#include "word.h"
#include <stdlib.h>
#include "code.h"

Word *word_create(uint8_t *syms, uint32_t len) {
	Word *word = malloc(sizeof(Word));
	if (word == NULL) {
		return NULL;
	}
	word->syms = malloc(len);
	for (uint32_t i = 0; i < len; i++) {
		word->syms[i] = syms[i];
	}
	word->len = len;
	return word;
}

Word *word_append_sym(Word *w, uint8_t sym) {
	Word *word = word_create(w->syms, w->len);
	word->len = word->len + 1;
	word->syms = realloc(word->syms, word->len);
	word->syms[word->len - 1] = sym;
	return word;
}

void word_delete(Word *w) {
	free(w->syms);
	free(w);
}

WordTable *wt_create(void) {
	WordTable *wt = malloc(MAX_CODE * sizeof(Word *));	
	for (int i = 0; i < MAX_CODE; i++) {
		wt[i] = NULL;
	}
	wt[EMPTY_CODE] = word_create(NULL, 0);
	return wt;
}

void wt_reset(WordTable *wt) {
	for (int i = 0; i < MAX_CODE; i++) {
		if (i != EMPTY_CODE) {
			word_delete(wt[i]);
			wt[i] = NULL;
		}
	}
}

void wt_delete(WordTable *wt) {
	free(wt);
}

