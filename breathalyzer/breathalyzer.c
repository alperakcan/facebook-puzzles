
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "fileio.h"
#include "stringtable.h"

#if !defined(DICTIONARY_FILE)
#define DICTIONARY_FILE "/var/tmp/twl06.txt"
#endif

typedef struct dic_pack_s {
	unsigned int nwords;
	unsigned int swords;
	char **words;
} dic_pack_t;

static unsigned int n_dic_packs = 0;
static dic_pack_t **dic_packs = NULL;
static stringtable_t *dic_hash = NULL;

static inline int string_tolower (char *str)
{
	while (*str) {
		*str = alloc_tolower(*str);
		str++;
	}
	return 0;
}

/* implementation of levenshtein distance
 */
static inline int calculate_step (int **matrix, char *input, int inputl, char *word, int wordl)
{
	int d;
	int e;
	int f;
	int i;
	int *v0;
	int *v1;
	int *vt;
	int *v0t;
	int *v1t;
	char *in;
	char *wo;
	v0 = matrix[0];
	v1 = matrix[1];
	for (i = 0; i <= wordl; i++) {
		v0[i] = i;
	}
	for (in = input; *in; in++) {
		v1[0] = in - input + 1;
		for (wo = word, v0t = v0, v1t = v1; *wo; wo++) {
			f = *v0t++ + ((*in == (*wo)) ? 0 : 1);
			e = *v0t + 1;
			d = *v1t++ + 1;
			*v1t = MIN3(d, e, f);
		}
		vt = v0;
		v0 = v1;
		v1 = vt;
	}
	return v0[wordl];
}

int main (int argc, char *argv[])
{
	unsigned int n;
	unsigned int i;
	unsigned int r;
	unsigned int t;
	unsigned int s;
	unsigned int inputl;
	unsigned int wordl;
	unsigned int words;
	unsigned int wordb;
	int *matrix[2];
	char *input;
	char *word;
	fileio_t *fio;
	fileio_t *dic;
	char **t_dic_words;
	dic_pack_t **t_dic_packs;
	dic_pack_t **ts_dic_packs;
	dic_pack_t **tb_dic_packs;
	char *dicfile = DICTIONARY_FILE;
	if (argc != 2) {
		errorf("invalid argument count: %d, should be 2", argc);
	}
	n = 0;
	r = 0;
	fio = fileio_open(argv[1]);
	dic = fileio_open(dicfile);
	n_dic_packs = 256;
	dic_packs = alloc_malloc(sizeof(dic_pack_t *) * n_dic_packs);
	memset(dic_packs, 0, sizeof(dic_pack_t *) * n_dic_packs);
	dic_hash = stringtable_init();
	while (1) {
		word = fileio_read_string(dic, &wordl);
		if (word == NULL) {
			break;
		}
		n = MAX(n, wordl + 1);
		if (n_dic_packs <= wordl) {
			t_dic_packs = alloc_malloc(sizeof(dic_pack_t *) * (wordl + 1));
			memset(t_dic_packs, 0, sizeof(dic_pack_t *) * (wordl + 1));
			memcpy(t_dic_packs, dic_packs, sizeof(dic_pack_t *) * n_dic_packs);
			n_dic_packs = wordl + 1;
			alloc_free(dic_packs);
			dic_packs = t_dic_packs;
		}
		if (dic_packs[wordl] == NULL) {
			dic_packs[wordl] = alloc_malloc(sizeof(dic_pack_t));
			dic_packs[wordl]->nwords = 0;
			dic_packs[wordl]->swords = 256;
			dic_packs[wordl]->words = alloc_malloc(sizeof(char *) * dic_packs[wordl]->swords);
		}
		string_tolower(word);
		if (dic_packs[wordl]->nwords >= dic_packs[wordl]->swords) {
			dic_packs[wordl]->swords += 256;
			t_dic_words = alloc_malloc(sizeof(char *) * (dic_packs[wordl]->swords));
			memcpy(t_dic_words, dic_packs[wordl]->words, sizeof(char *) * dic_packs[wordl]->nwords);
			alloc_free(dic_packs[wordl]->words);
			dic_packs[wordl]->words = t_dic_words;
		}
		dic_packs[wordl]->words[dic_packs[wordl]->nwords] = word;
		dic_packs[wordl]->nwords += 1;
#if 1
		stringtable_insert(dic_hash, word, (void *) 1, NULL);
#endif
	}
	fileio_close(dic);
	matrix[0] = alloc_malloc(sizeof(int) * n);
	matrix[1] = alloc_malloc(sizeof(int) * n);
	while (1) {
		input = fileio_read_string(fio, &inputl);
		if (input == NULL) {
			break;
		}
#if 1
		if (stringtable_find(dic_hash, input)) {
			alloc_free(input);
			continue;
		}
#endif
		s = ~0;
		if (inputl < n_dic_packs) {
			wordb = inputl;
			words = inputl - 1;
		} else {
			wordb = n_dic_packs - 1;
			words = n_dic_packs - 2;
		}
		for (ts_dic_packs = &dic_packs[words], tb_dic_packs = &dic_packs[wordb];
		     s && words > 0 && wordb < n_dic_packs;
		     words--, ts_dic_packs--, wordb++, tb_dic_packs++) {
			if (abs(words - inputl) > s) {
				break;
			}
			if ((*ts_dic_packs) != NULL &&
			    (*ts_dic_packs)->nwords > 0) {
				for (i = 0, t_dic_words = &(*ts_dic_packs)->words[0];
				     s && i < (*ts_dic_packs)->nwords;
				     i++, t_dic_words++) {
					if (abs(words - inputl) > s) {
						/* we will have to do at least (wordl - l) changes */
						break;
					}
					t = calculate_step(matrix, input, inputl, *t_dic_words, words);
					s = MIN(s, t);
				}
			}
			if (abs(wordb - inputl) > s) {
				break;
			}
			if ((*tb_dic_packs) != NULL &&
			    (*tb_dic_packs)->nwords > 0) {
				for (i = 0, t_dic_words = &(*tb_dic_packs)->words[0];
				     s && i < (*tb_dic_packs)->nwords;
				     i++, t_dic_words++) {
					if (abs(wordb - inputl) > s) {
						/* we will have to do at least (wordl - l) changes */
						break;
					}
					t = calculate_step(matrix, input, inputl, *t_dic_words, wordb);
					s = MIN(s, t);
				}
			}
		}
		for (tb_dic_packs = &dic_packs[wordb];
		     s && wordb < n_dic_packs;
		     wordb++, tb_dic_packs++) {
			if (abs(wordb - inputl) > s) {
				break;
			}
			if ((*tb_dic_packs) != NULL &&
			    (*tb_dic_packs)->nwords > 0) {
				for (i = 0, t_dic_words = &(*tb_dic_packs)->words[0];
				     s && i < (*tb_dic_packs)->nwords;
				     i++, t_dic_words++) {
					if (abs(wordb - inputl) > s) {
						/* we will have to do at least (wordl - l) changes */
						break;
					}
					t = calculate_step(matrix, input, inputl, *t_dic_words, wordb);
					s = MIN(s, t);
				}
			}
		}
		for (ts_dic_packs = &dic_packs[words];
		     s && words > 0;
		     words--, ts_dic_packs--) {
			if (abs(words - inputl) > s) {
				break;
			}
			if ((*ts_dic_packs) != NULL &&
			    (*ts_dic_packs)->nwords > 0) {
				for (i = 0, t_dic_words = &(*ts_dic_packs)->words[0];
				     s && i < (*ts_dic_packs)->nwords;
				     i++, t_dic_words++) {
					if (abs(words - inputl) > s) {
						/* we will have to do at least (wordl - l) changes */
						break;
					}
					t = calculate_step(matrix, input, inputl, *t_dic_words, words);
					s = MIN(s, t);
				}
			}
		}
		alloc_free(input);
		r += s;
	}
	fileio_close(fio);
	for (wordl = 0, t_dic_packs = &dic_packs[0]; wordl < n_dic_packs; wordl++, t_dic_packs++) {
		if ((*t_dic_packs) == NULL) {
			continue;
		}
		for (i = 0, t_dic_words = &(*t_dic_packs)->words[0]; i < (*t_dic_packs)->nwords; i++, t_dic_words++) {
			alloc_free(*t_dic_words);
		}
		alloc_free((*t_dic_packs)->words);
		alloc_free(*t_dic_packs);
	}
	stringtable_uninit(dic_hash);
	alloc_free(dic_packs);
	alloc_free(matrix[0]);
	alloc_free(matrix[1]);
	printf("%d\n", r);
	return 0;
}
