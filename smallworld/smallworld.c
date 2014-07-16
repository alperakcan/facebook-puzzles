
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "fileio.h"

typedef struct point_s {
	unsigned int id;
	double lat;
	double lon;
} point_t;

typedef struct kdtree_s {
	point_t *point;
	struct kdtree_s *left;
	struct kdtree_s *right;
} kdtree_t;

static inline double point_distance (point_t *p0, point_t *p1)
{
	double lat = p0->lat - p1->lat;
	double lon = p0->lon - p1->lon;
	return (lat * lat) + (lon * lon);
}

int main (int argc, char *argv[])
{
	fileio_t *fio;
	unsigned int i;
	double lat;
	double lon;
	if (argc != 2) {
		errorf("invalid argument count: %d, should be 2", argc);
	}
	fio = fileio_open(argv[1]);
	while (!fileio_eof(fio)) {
		i = fileio_read_ui(fio);
		lat = fileio_read_d(fio);
		lon = fileio_read_d(fio);
		debugf("%u %f %f", i, lat, lon);
	}
	fileio_close(fio);
	return 0;
}
