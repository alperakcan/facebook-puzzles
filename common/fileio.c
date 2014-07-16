/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "common.h"
#include "fileio.h"

struct fileio_s {
	int fd;
	FILE *fp;
	off_t size;
	char *buffer;
	char *ptr;
	char *end;
};

fileio_t * fileio_open (const char *path)
{
	int r;
	int fd;
	FILE *fp;
	void *addr;
	fileio_t *fio;
	struct stat stbuf;
	r = stat(path, &stbuf);
	if (r != 0) {
		errorf("stat(%s) failed", path);
		return NULL;
	}
	fp = fopen(path, "r");
	if (fp == NULL) {
		errorf("fopen(%s) failed", path);
		return NULL;
	}
	fd = fileno(fp);
	addr = mmap(0, stbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		errorf("mmap() failed");
		fclose(fp);
		return NULL;
	}
	fio = alloc_malloc(sizeof(*fio));
	fio->size = stbuf.st_size;
	fio->fp = fp;
	fio->buffer = addr;
	fio->ptr = addr;
	fio->end = addr + stbuf.st_size;
	return fio;
}

int fileio_close (fileio_t *fio)
{
	if (fio == NULL) {
		return 0;
	}
	munmap(fio->buffer, fio->size);
	fclose(fio->fp);
	alloc_free(fio);
	return 0;
}

int fileio_seek_start (fileio_t *fio)
{
	fio->ptr = fio->buffer;
	return 0;
}

int fileio_seek_pos (fileio_t *fio, unsigned int pos)
{
	fio->ptr += pos;
	return 0;
}

static inline int fileio_isdigit (char c)
{
	if (c >= '0' &&
	    c <= '9') {
		return 1;
	}
	return 0;
}

static inline int fileio_isalpha (char c)
{
	if (c >= 'a' &&
	    c <= 'z') {
		return 1;
	}
	if (c >= 'A' &&
	    c <= 'Z') {
		return 1;
	}
	return 0;
}

static inline int fileio_isalnum (char c)
{
	if (fileio_isdigit(c) ||
	    fileio_isalpha(c)) {
		return 1;
	}
	return 0;
}

char * fileio_get_string (fileio_t *fio, unsigned int *length)
{
	char *ptr;
	char *ret;
	unsigned long long len;
	for (; fio->ptr < fio->end; fio->ptr++) {
		if (fileio_isalnum(*fio->ptr)) {
			break;
		}
	}
	for (ptr = fio->ptr; ptr < fio->end; ptr++) {
		if (!fileio_isalnum(*ptr)) {
			break;
		}
	}
	len = ptr - fio->ptr;
	if (!len) {
		return NULL;
	}
	*length = len;
	ret = fio->ptr;
	fio->ptr = ptr;
	return ret;
}

char * fileio_read_string (fileio_t *fio, unsigned int *length)
{
	char *ptr;
	char *ret;
	unsigned long long len;
	for (; fio->ptr < fio->end; fio->ptr++) {
		if (fileio_isalnum(*fio->ptr)) {
			break;
		}
	}
	for (ptr = fio->ptr; ptr < fio->end; ptr++) {
		if (!fileio_isalnum(*ptr)) {
			break;
		}
	}
	len = ptr - fio->ptr;
	if (!len) {
		*length = 0;
		return NULL;
	}
	ret = alloc_malloc(len + 1);
	memcpy(ret, fio->ptr, len);
	*(ret + len) = '\0';
	*length = len;
	fio->ptr = ptr;
	return ret;
}

unsigned int fileio_read_ui (fileio_t *fio)
{
	char *ptr;
	unsigned int ret = 0;
	for (; fio->ptr < fio->end; fio->ptr++) {
		if (fileio_isdigit(*fio->ptr)) {
			break;
		}
	}
	for (ptr = fio->ptr; ptr < fio->end; ptr++) {
		if (!fileio_isdigit(*ptr)) {
			break;
		}
		ret = (10 * ret) + (*ptr - '0');
	}
	fio->ptr = ptr;
	return ret;
}

double fileio_read_d (fileio_t *fio)
{
	char *ptr;
	double ret;
	for (; fio->ptr < fio->end; fio->ptr++) {
		if (fileio_isdigit(*fio->ptr) ||
		    *fio->ptr == '-') {
			break;
		}
	}
	ret = atof(fio->ptr);
	for (ptr = fio->ptr; ptr < fio->end; ptr++) {
		if (*ptr == '.') {
			break;
		}
	}
	fio->ptr = ptr + 1;
	for (ptr = fio->ptr; ptr < fio->end; ptr++) {
		if (!fileio_isdigit(*ptr)) {
			break;
		}
	}
	fio->ptr = ptr;
	return ret;
}

int fileio_eof (fileio_t *fio)
{
	for (; fio->ptr < fio->end; fio->ptr++) {
		if (fileio_isalnum(*fio->ptr)) {
			break;
		}
	}
	if (fio->ptr >= fio->end) {
		return 1;
	}
	return 0;
}
