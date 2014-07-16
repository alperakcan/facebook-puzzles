
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
	int rc;
	int p;
	long long int i;
	long long int j;
	long long int siz;
	long long int len;
	FILE *fp;
	char *input;
	char *buffer;
	char *tmp;
	struct stat stbuf;
	if (argc != 2) {
		fprintf(stderr, "invalid argument count: %dm should be 2\n", argc);
		exit(-1);
	}
	input = argv[1];
	rc = stat(input, &stbuf);
	if (rc != 0) {
		fprintf(stderr, "can not get stat for file: %s\n", input);
		exit(-2);
	}
	len = stbuf.st_size;
	buffer = (char *) malloc(len + 1);
	if (buffer == NULL) {
		fprintf(stderr, "can not allocate memory of size: %lld\n", (long long int) len);
		exit(-3);
	}
	fp = fopen(input, "r");
	if (fp == NULL) {
		fprintf(stderr, "can not open file: %s\n", input);
		exit(-4);
	}
	siz = fread(buffer, 1, len, fp);
	if (siz != len) {
		fprintf(stderr, "can not read %lld bytes (%lld)\n", len, siz);
		exit(-4);
	}
	buffer[siz + 1] = '\0';
	for (tmp = buffer + siz; tmp >= buffer; tmp--) {
		if (*tmp < '0' || *tmp > '9') {
			*tmp = '\0';
		} else {
			break;
		}
	}
	for (tmp = buffer; *tmp; tmp++) {
		if (!(*tmp < '0' || *tmp > '9')) {
			break;
		}
	}
	j = atoll(tmp);
	for (i = 1; i <= j; i++) {
		p = !(i % 3);
		p |= (!(i % 5)) << 1;
		if (p == 0) {
			continue;
		} else if (p == 1) {
			printf("Hoppity\n");
		} else if (p == 2) {
			printf("Hophop\n");
		} else {
			printf("Hop\n");
		}
	}
	free(buffer);
	fclose(fp);
	return 0;
}
