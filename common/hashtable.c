/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "hashtable.h"

struct hashtable_node_s {
	unsigned int key;
	char *name;
	void *data;
};

struct hashtable_table_s {
	hashtable_table_t *table;
	unsigned int snodes;
	unsigned int nnodes;
	hashtable_node_t *nodes;
};

struct hashtable_s {
	unsigned int size;
	unsigned int mask;
	unsigned int mlen;
	unsigned int depth;
	hashtable_table_t *table;
};

static hashtable_node_t * hashtable_get_node (hashtable_t *htable, char *name);
static int hashtable_uninit_table (hashtable_table_t *table, unsigned int size);

static inline int hashtable_cmp_string (char *str, char *ptr)
{
	while (*str && *ptr) {
		if (*str++ != *ptr++) {
			return -1;
		}
	}
	if (*str || *ptr) {
		return -1;
	}
	return 0;
}

static inline unsigned int hashtable_hash_string (const char *str)
{
	#define HASHWORDBITS 32
	unsigned int hval, g;
	hval = 0;
	while (*str) {
		hval <<= 4;
		hval += (unsigned char) *str++;
		g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
		if (g) {
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	return hval;
}

static inline hashtable_node_t * hashtable_get_node (hashtable_t *htable, char *name)
{
	unsigned int o;
	unsigned int l;
	unsigned int m;
	unsigned int s;
	unsigned int h;
	unsigned int mlen;
	unsigned int mask;
	unsigned int depth;
	hashtable_node_t *node;
	hashtable_table_t *tmp;
	mask = htable->mask;
	mlen = htable->mlen;
	depth = htable->depth;
	h = hashtable_hash_string(name);
	tmp = htable->table;
	for (l = 0; l < depth; l++) {
		o = h & mask;
		if (tmp[o].table == NULL) {
			return NULL;
		}
		tmp = tmp[o].table;
		h >>= mlen;
		if (h == 0) {
			break;
		}
	}
	o = 0;
	l = tmp[h & mask].nnodes;
	s = l;
	if (l == 0) {
		return NULL;
	}
	node = tmp[h & mask].nodes;
	while (l >= o) {
		m = (o + l) >> 1;
		if (m >= s) {
			return NULL;
		}
		if (h == node[m].key) {
			while (m > 0) {
				if (h == node[m - 1].key) {
					m--;
				} else {
					break;
				}
			}
			goto found;
		} else if (h < node[m].key) {
			l = m - 1;
		} else {
			o = m + 1;
		}
	}
	return NULL;
found:
	l = tmp[h & mask].nnodes;
	node = tmp[h & mask].nodes;
	while (m < l) {
		if (h != node[m].key) {
			break;
		}
		if (hashtable_cmp_string(node[m].name, name) == 0) {
			return &node[m];
		}
		m++;
	}
	return NULL;
}

void * hashtable_add (hashtable_t *htable, char *name, void *data)
{
	unsigned int o;
	unsigned int l;
	unsigned int s;
	unsigned int h;
	unsigned int m;
	unsigned int key;
	unsigned int mlen;
	unsigned int mask;
	unsigned int depth;
	hashtable_node_t *node;
	hashtable_table_t *tmp;
	mask = htable->mask;
	mlen = htable->mlen;
	depth = htable->depth;
	key = hashtable_hash_string(name);
	h = key & mask;
	tmp = htable->table;
	for (l = 0; l < depth; l++) {
		o = h;
		if (tmp[o].table == NULL) {
			tmp[o].table = (hashtable_table_t *) alloc_malloc(sizeof(hashtable_t) * htable->size);
			memset(tmp[o].table, 0, sizeof(hashtable_table_t) * htable->size);
		}
		tmp = tmp[o].table;
		h >>= mlen;
		if (h == 0) {
			break;
		}
	}
	o = 0;
	m = 0;
	l = tmp[h].nnodes;
	s = l;
	if (l == 0) {
		goto just_add;
	}
	if (key < tmp[h].nodes[0].key) {
		goto just_add;
	}
	node = tmp[h].nodes;
	while (l >= o) {
		m = (o + l) >> 1;
		if (m >= s) {
			break;
		}
		if (key == node[m].key) {
			while (m > 0) {
				if (key == node[m - 1].key) {
					m--;
				} else {
					break;
				}
			}
			while (m < l) {
				if (key != node[m].key) {
					break;
				}
				if (hashtable_cmp_string(node[m].name, name) == 0) {
					return node[m].data;
				}
				m++;
			}
			break;
		} else if (key < node[m].key) {
			l = m - 1;
		} else {
			o = m + 1;
		}
	}
	l = tmp[h].nnodes;
	if (m >= l) {
		m = l;
		goto just_add;
	}
	while (m > 0) {
		if (node[m - 1].key > key) {
			m--;
		} else {
			break;
		}
	}
	while (m < l) {
		if (node[m].key < key) {
			m++;
		} else {
			break;
		}
	}
just_add:
	o = tmp[h].snodes;
	if (l + 1 > o) {
		o = (o) ? o * 2 : 2;
		node = alloc_malloc(sizeof(hashtable_node_t) * o);
		memcpy(node, tmp[h].nodes, sizeof(hashtable_node_t) * l);
		alloc_free(tmp[h].nodes);
		tmp[h].nodes = node;
		tmp[h].snodes = o;
	}
	node = tmp[h].nodes;
	memmove(&node[m + 1], &node[m], sizeof(hashtable_node_t) * (l - m));
	node = &node[m];
	memset(node, 0, sizeof(hashtable_node_t));
	node->key = key;
	node->name = strdup(name);
	node->data = data;
	tmp[h].nnodes += 1;
	return data;
}

void * hashtable_get_data (hashtable_t *htable, char *name)
{
	hashtable_node_t *node;
	node = hashtable_get_node(htable, name);
	if (node) {
		return node->data;
	} else {
		return NULL;
	}
}

hashtable_t * hashtable_init (unsigned int mask, unsigned int depth)
{
	unsigned int l;
	hashtable_t *htbl;
	htbl = (hashtable_t *) alloc_malloc(sizeof(hashtable_t));
	htbl->size = mask + 1;
	htbl->mask = mask;
	htbl->depth = depth;
	for (l = 32; l > 0; l--) {
		if (mask & (1 << (l - 1))) {
			break;
		}
	}
	htbl->mlen = l;
	htbl->table = (hashtable_table_t *) alloc_malloc(sizeof(hashtable_table_t) * htbl->size);
	memset(htbl->table, 0, sizeof(hashtable_table_t) * htbl->size);
	return htbl;
}

static int hashtable_uninit_table (hashtable_table_t *table, unsigned int size)
{
	hashtable_node_t *node;
	unsigned int i;
	if (table == NULL) {
		return -1;
	}
	for (i = 0; i < size; i++) {
		hashtable_uninit_table(table[i].table, size);
		node = table[i].nodes;
		alloc_free(node);
	}
	alloc_free(table);
	return 0;
}

int hashtable_uninit (hashtable_t *htable)
{
	hashtable_uninit_table(htable->table, htable->size);
	alloc_free(htable);
	return 0;
}
