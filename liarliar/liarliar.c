/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "fileio.h"
#include "stringtable.h"

typedef struct liarliar_person_s {
	int liar;
	struct liarliar_person_s **liars;
	unsigned int nliars;
	unsigned int sliars;
} liarliar_person_t;

typedef struct liarliar_s {
	unsigned int size;
	unsigned int nmembers;
	stringtable_t *stringtable;
	liarliar_person_t *mempool;
} liarliar_t;

static liarliar_t liarliar;

static inline liarliar_person_t * liarliar_add_person (char *person)
{
	int already;
	liarliar_person_t *n;
	n = liarliar.mempool + liarliar.nmembers;
	n = stringtable_insert(liarliar.stringtable, person, n, &already);
	if (already) {
		return n;
	}
	n->liar = -1;
	n->sliars = 4;
	n->nliars = 0;
	n->liars = alloc_malloc(sizeof(liarliar_person_t *) * n->sliars);
	liarliar.nmembers += 1;
	return n;
}

static inline void liarliar_add_liar (liarliar_person_t *person, char *liar)
{
	liarliar_person_t *pl;
	liarliar_person_t **liars;
	pl = liarliar_add_person(liar);
	if (pl->nliars + 1 >= pl->sliars) {
		pl->sliars *= 2;
		liars = alloc_malloc(sizeof(liarliar_person_t *) * pl->sliars);
		memcpy(liars, pl->liars, sizeof(liarliar_person_t *) * pl->nliars);
		alloc_free(pl->liars);
		pl->liars = liars;
	}
	pl->liars[pl->nliars] = person;
	pl->nliars += 1;
	if (person->nliars + 1 >= person->sliars) {
		person->sliars *= 2;
		liars = alloc_malloc(sizeof(liarliar_person_t *) * person->sliars);
		memcpy(liars, person->liars, sizeof(liarliar_person_t *) * person->nliars);
		alloc_free(person->liars);
		person->liars = liars;
	}
	person->liars[person->nliars] = pl;
	person->nliars += 1;
}

int main (int argc, char *argv[])
{
	fileio_t *fio;
	char *liar;
	char *person;
	unsigned int n;
	unsigned int j;
	unsigned int npq;
	unsigned int liars;
	liarliar_person_t *p;
	liarliar_person_t **pp;
	liarliar_person_t **pq;
	if (argc != 2) {
		errorf("invalid argument count: %d, should be 2", argc);
	}
	fio = fileio_open(argv[1]);
	p = NULL;
	liarliar.size = fileio_read_ui(fio);
	debugf("liarliar.size: %d", liarliar.size);
	liarliar.nmembers = 0;
	liarliar.stringtable = stringtable_init();
	liarliar.mempool = alloc_malloc(sizeof(liarliar_person_t) * liarliar.size);
	while (1) {
		person = fileio_read_string(fio, &j);
		if (!person) {
			break;
		}
		p = liarliar_add_person(person);
		n = fileio_read_ui(fio);
		while (n--) {
			liar = fileio_read_string(fio, &j);
			liarliar_add_liar(p, liar);
			alloc_free(liar);
		}
		alloc_free(person);
	}
	liars = 0;
	pq = NULL;
	if (!p) {
		goto out;
	}
	pq = alloc_malloc(sizeof(liarliar_person_t *) * liarliar.size);
	pq[0] = p;
	npq = 1;
	p->liar = 1;
	liars = 1;
	while (npq) {
		p = *pq;
		for (j = p->nliars, pp = &p->liars[0]; j--; pp++) {
			if ((*pp)->liar != -1) {
				continue;
			}
			if (((*pp)->liar = !p->liar)) {
				liars++;
			}
			pq[npq] = *pp;
			npq += 1;
		}
		memmove(&pq[0], &pq[1], sizeof(liarliar_person_t *) * (npq - 1));
		npq -= 1;
	}
out:	stringtable_uninit(liarliar.stringtable);
	for (n = 0; n < liarliar.nmembers; n++) {
		p = liarliar.mempool + n;
		alloc_free(p->liars);
	}
	alloc_free(liarliar.mempool);
	alloc_free(pq);
	fileio_close(fio);
	printf("%u %u\n", MAX(liarliar.size - liars, liars), MIN(liarliar.size - liars, liars));
	return 0;
}
