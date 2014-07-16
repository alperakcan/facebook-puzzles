
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "fileio.h"

typedef struct node_s {
	unsigned int start;
	unsigned int end;
	unsigned int weight;
	unsigned int link;
	unsigned int score;
} node_t;

unsigned int snodes;
static node_t **nodes;
static node_t *pool;

#define _0(v) (unsigned) ((v)         & 0x7FF)
#define _1(v) (unsigned) (((v) >> 11) & 0x7FF)
#define _2(v) (unsigned) ((v)  >> 22)
#define HIST_SIZE 2048

static inline void radix_sort (node_t **src, node_t **dst, const long sz)
{
	long j;
	unsigned pos;
	long n, sum0 = 0 , sum1 = 0 , sum2 = 0 , tsum = 0;
	node_t **reader, **writer, **buf = dst;
	size_t *b0, *b1, *b2;
	b0   = alloc_malloc(HIST_SIZE * 3 * sizeof(size_t));
	b1   = b0 + HIST_SIZE;
	b2   = b1 + HIST_SIZE;

	memset(b0, 0, 3 * HIST_SIZE * sizeof(size_t));
	for (n=0; n < sz; n++) {
		b0[_0(src[n]->start)]++;
		b1[_1(src[n]->start)]++;
		b2[_2(src[n]->start)]++;
	}
	for (j = 0; j < HIST_SIZE; j++) {
		tsum  = b0[j] + sum0;
		b0[j] = sum0 - 1;
		sum0  = tsum;

		tsum  = b1[j] + sum1;
		b1[j] = sum1 - 1;
		sum1  = tsum;

		tsum  = b2[j] + sum2;
		b2[j] = sum2 - 1;
		sum2  = tsum;
	}

	writer = buf;
	reader = src;
	for (n=0; n < sz; n++) {
		pos = _0(reader[n]->start);
		writer[++b0[pos]] = reader[n];
	}
	writer = src;
	reader = buf;
	for (n=0; n < sz; n++) {
		pos = _1(reader[n]->start);
		writer[++b1[pos]] = reader[n];
	}
	writer = buf;
	reader = src;
	for (n=0; n < sz; n++) {
		pos = _2(reader[n]->start);
		writer[++b2[pos]] = reader[n];
	}
	alloc_free(b0);
}

int main (int argc, char *argv[])
{
	unsigned int h;
	unsigned int nos;
	unsigned int hos;
	unsigned int score;
	unsigned int l;
	unsigned int r;
	unsigned int m;
	unsigned int i;
	unsigned int n;
	unsigned int s;
	unsigned int e;
	unsigned int w;
	unsigned int ndna;
	node_t *node;
	node_t **tnodes;
	fileio_t *fio;
	if (argc != 2) {
		errorf("invalid argument count: %d, should be 2", argc);
	}
	fio = fileio_open(argv[1]);
	ndna = fileio_read_ui(fio);
	fileio_seek_pos(fio, ndna + (ndna / 80));
	snodes = fileio_read_ui(fio);
	debugf("ndna: %u, npre: %u", ndna, snodes);
	pool = alloc_malloc(sizeof(node_t) * (ndna + 1));
	nodes = alloc_malloc(sizeof(node_t *) * (ndna + 1));
	for (n = 0; n < snodes; n++) {
		s = fileio_read_ui(fio);
		e = fileio_read_ui(fio);
		w = fileio_read_ui(fio);
		node = pool + n;
		node->start = s;
		node->end = e;
		node->weight = w;
		node->link = 0;
		node->score = 0;
		nodes[n] = node;
	}
#if defined(DEBUG)
	debugf("before sort");
	for (n = 0; n < snodes; n++) {
		debugf("  s: %u, e: %u, w: %u", nodes[n]->start, nodes[n]->end, nodes[n]->weight);
	}
#endif
	tnodes = alloc_malloc(sizeof(node_t *) * (snodes + 1));
	radix_sort(nodes, tnodes, snodes);
	alloc_free(nodes);
	nodes = tnodes;
#if defined(DEBUG)
	debugf("after sort");
	for (n = 0; n < snodes; n++) {
		debugf("  s: %u, e: %u, w: %u", nodes[n]->start, nodes[n]->end, nodes[n]->weight);
	}
#endif
	for (n = 0; n < snodes; n++) {
		e = nodes[n]->end;
		m = 0;
		l = n + 1;
		r = snodes;
		while (r >= l) {
			m = (l + r) >> 1;
			if (m >= snodes) {
				break;
			}
			if (e == nodes[m]->start) {
				break;
			} else if (e < nodes[m]->start) {
				r = m - 1;
			} else {
				l = m + 1;
			}
		}
		while (m < snodes) {
			if (nodes[m]->start <= e) {
				m++;
			} else {
				break;
			}
		}
		nodes[n]->link = m - 1;
	}
#if defined(DEBUG) && 0
	debugf("after link");
	for (n = 0; n < snodes; n++) {
		debugf("  s: %u, e: %u, w: %u, l: %u", nodes[n]->start, nodes[n]->end, nodes[n]->weight, nodes[n]->link);
	}
#endif
	h = 0;
	hos = 0;
	score = 0;
	for (n = 0; n < snodes; n++) {
		node = nodes[n];
		if (node->link  > hos) {
			h = nodes[hos]->score;
			for (i = hos + 1; i <= node->link; i++) {
				nodes[i]->score = h;
			}
			hos = node->link;
		}
		nos = score + node->weight;
		if (nos > nodes[node->link]->score) {
			for (i = node->link; i <= hos; i++) {
				if (nos > nodes[i]->score) {
					nodes[i]->score = nos;
				}
			}
		}
		score = node->score;
	}
	fileio_close(fio);
	alloc_free(nodes);
	alloc_free(pool);
	printf("%u\n", score);
	return 0;
}
