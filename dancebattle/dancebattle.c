
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "fileio.h"

typedef struct move_s {
	unsigned int first;
	unsigned int second;
} move_t;

unsigned char ** valid_moves (unsigned int n, unsigned char **played, move_t *last)
{
	unsigned int i;
	unsigned int j;
	unsigned int r;
	unsigned char **moves;
	moves = alloc_malloc(sizeof(*moves) * n);
	for (i = 0; i < n; i++) {
		moves[i] = alloc_malloc(sizeof(*moves[i]) * n);
		memset(moves[i], 0, sizeof(*moves[i]) * n);
	}
	if (last == NULL) {
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				moves[i][j] = 1;
			}
		}
	} else {
		r = 0;
		for (i = 0; i < n; i++) {
			if (played[last->second][i] == 0) {
				moves[last->second][i] = 1;
				r = 1;
			}
		}
		if (r == 0) {
			for (i = 0; i < n; i++) {
				alloc_free(moves[i]);
			}
			alloc_free(moves);
			moves = NULL;
		}
	}
	return moves;
}

int minimax_alpha_beta (unsigned int n, unsigned int m, unsigned char **played, move_t *last, int alpha, int beta)
{
	int r;
	int b;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned char **moves;
	move_t move;
	moves = valid_moves(n, played, last);
	if (moves == NULL) {
		return m ? -1 : 1;
	}
	b = m ? alpha : beta;
	i = 0;
	k = n;
	while (i < k) {
		if (last) {
			i = last->second;
		}
		for (j = 0; j < n; j++) {
			if (moves[i][j] == 1) {
				played[i][j] = 1;
				played[j][i] = 1;
				move.first = i;
				move.second = j;
				r = minimax_alpha_beta(n, !m, played, &move, alpha, beta);
				played[i][j] = 0;
				played[j][i] = 0;
				if (m) {
					if (r > b) {
						b = r;
						alpha = r;
					}
					if (b >= 1) {
						goto out;
					}
				} else {
					if (r < b) {
						b = r;
						beta = r;
					}
					if (b <= -1) {
						goto out;
					}
				}
				if (alpha >= beta) {
					goto out;
				}
			}
		}
		i++;
		if (last) {
			break;
		}
	}
out:
	for (i = 0; i < n; i++) {
		alloc_free(moves[i]);
	}
	alloc_free(moves);
	return b;
}

int main (int argc, char *argv[])
{
	fileio_t *fio;
	int r;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int m;
	unsigned int smoves;
	unsigned int nmoves;
	unsigned char **played;
	move_t last;
	if (argc != 2) {
		errorf("invalid argument count: %d, should be 2", argc);
	}
	fio = fileio_open(argv[1]);
	smoves = fileio_read_ui(fio);
	nmoves = fileio_read_ui(fio);
	played = alloc_malloc(sizeof(*played) * smoves);
	for (i = 0; i < smoves; i++) {
		played[i] = alloc_malloc(sizeof(*played[i]) * smoves);
		memset(played[i], 0, sizeof(*played[i]) * smoves);
	}
	j = 0;
	k = 0;
	for (i = 0; i < nmoves; i++) {
		j = fileio_read_ui(fio);
		k = fileio_read_ui(fio);
		played[j][k] = 1;
		played[k][j] = 1;
		last.first = j;
		last.second = k;
	}
	m = (nmoves % 2) == 0;
	fileio_close(fio);
	r = minimax_alpha_beta(smoves, m, played, (nmoves > 0) ? &last : NULL, -2, 2);
	printf("%s\n", (r == 1) ? "Win" : "Lose");
	return 0;
}
