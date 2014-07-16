
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "stringtable.h"

typedef struct stringnode_s {
	unsigned int hash;
	char *string;
	void *data;
} stringnode_t;

typedef struct stringarray_s {
	unsigned int selements;
	unsigned int nelements;
	stringnode_t **nodes;
} stringarray_t;

struct stringtable_s {
	unsigned int sarrays;
	stringarray_t *arrays;
};

int stringtable_uninit (stringtable_t *stringtable)
{
	unsigned int i;
	unsigned int j;
	stringarray_t *a;
	stringnode_t **n;
	for (i = stringtable->sarrays, a = stringtable->arrays; i--; a++) {
		for (j = a->nelements, n = a->nodes; j--; n++) {
			alloc_free((*n)->string);
			alloc_free(*n);
		}
		alloc_free(a->nodes);
	}
	alloc_free(stringtable->arrays);
	alloc_free(stringtable);
	return 0;
}

stringtable_t * stringtable_init (void)
{
	stringtable_t *s;
	debugf("stringtable init");
	s = alloc_malloc(sizeof(stringtable_t));
	s->sarrays = 0xffff;
	s->arrays = alloc_malloc(sizeof(stringarray_t) * s->sarrays);
	memset(s->arrays, 0, sizeof(stringarray_t) * s->sarrays);
	return s;
}

void * stringtable_find (stringtable_t *stringtable, char *string)
{
	unsigned int d;
	unsigned int i;
	unsigned int h;
	unsigned int p;
	unsigned int l;
	unsigned int r;
	unsigned int m;
	unsigned int s;
	unsigned int n;
	stringnode_t **nodes;
	stringarray_t *array;
	h = alloc_hash(string);
	d = h % stringtable->sarrays;
	array = &stringtable->arrays[d];
	s = array->selements;
	n = array->nelements;
	if (!s) {
		p = 0;
		goto skip_search;
	}
	nodes = array->nodes;
	s = array->selements;
	n = array->nelements;
	if (h < nodes[0]->hash) {
		p = 0;
		goto skip_search;
	} else if (h == nodes[0]->hash) {
		p = 0;
	} else if (h > nodes[n - 1]->hash) {
		p = n;
		goto skip_search;
	} else if (h == nodes[n - 1]->hash) {
		p = n - 1;
		while (p > 0) {
			if (h == nodes[p - 1]->hash) {
				p--;
			} else {
				break;
			}
		}
	} else {
		l = 0;
		r = n;
		p = r - 1;
		while (r >= l) {
			m = (l + r) >> 1;
			if (h == nodes[m]->hash) {
				while (m > 0) {
					if (h == nodes[m - 1]->hash) {
						m--;
					} else {
						break;
					}
				}
				p = m;
				break;
			} else if (h < nodes[m]->hash) {
				r = m - 1;
			} else {
				l = m + 1;
			}
		}
		while (m < n) {
			if (nodes[m]->hash < h) {
				m++;
			} else {
				break;
			}
		}
		p = m;
	}
	for (i = p; i < n; i++) {
		if (nodes[i]->hash != h) {
			break;
		}
		if (alloc_strcmp(nodes[i]->string, string) == 0) {
			return nodes[i]->data;
		}
	}
skip_search:
	return NULL;
}

void * stringtable_insert (stringtable_t *stringtable, char *string, void *data, int *already)
{
	unsigned int d;
	unsigned int i;
	unsigned int h;
	unsigned int p;
	unsigned int l;
	unsigned int r;
	unsigned int m;
	unsigned int s;
	unsigned int n;
	stringnode_t *node;
	stringnode_t **nodes;
	stringarray_t *array;
	h = alloc_hash(string);
	d = h % stringtable->sarrays;
	array = &stringtable->arrays[d];
	s = array->selements;
	n = array->nelements;
	if (!s) {
		p = 0;
		array->selements = 1;
		array->nodes = alloc_malloc(sizeof(stringarray_t *) * (array->selements));
		nodes = array->nodes;
		goto skip_search;
	}
	nodes = array->nodes;
	s = array->selements;
	n = array->nelements;
	if (h < nodes[0]->hash) {
		p = 0;
		goto skip_search;
	} else if (h == nodes[0]->hash) {
		p = 0;
	} else if (h > nodes[n - 1]->hash) {
		p = n;
		goto skip_search;
	} else if (h == nodes[n - 1]->hash) {
		p = n - 1;
		while (p > 0) {
			if (h == nodes[p - 1]->hash) {
				p--;
			} else {
				break;
			}
		}
	} else {
		l = 0;
		r = n;
		p = r - 1;
		while (r >= l) {
			m = (l + r) >> 1;
			if (h == nodes[m]->hash) {
				while (m > 0) {
					if (h == nodes[m - 1]->hash) {
						m--;
					} else {
						break;
					}
				}
				p = m;
				break;
			} else if (h < nodes[m]->hash) {
				r = m - 1;
			} else {
				l = m + 1;
			}
		}
		while (m < n) {
			if (nodes[m]->hash < h) {
				m++;
			} else {
				break;
			}
		}
		p = m;
	}
	for (i = p; i < n; i++) {
		if (nodes[i]->hash != h) {
			break;
		}
		if (alloc_strcmp(nodes[i]->string, string) == 0) {
			if (already) {
				*already = 1;
			}
			return nodes[i]->data;
		}
	}
skip_search:
	if (array->nelements + 1 >= array->selements) {
		array->selements *= 2;
		nodes = alloc_malloc(sizeof(stringnode_t *) * (array->selements));
		memcpy(nodes, array->nodes, sizeof(stringnode_t *) * (array->nelements));
		alloc_free(array->nodes);
		array->nodes = nodes;
	}
	memmove(&nodes[p + 1], &nodes[p], sizeof(stringnode_t *) * (n - p));
	node = alloc_malloc(sizeof(stringnode_t));
	node->string = strdup(string);
	node->hash = h;
	node->data = data;
	nodes[p] = node;
	array->nelements += 1;
	if (already) {
		*already = 0;
	}
	return node->data;
}
