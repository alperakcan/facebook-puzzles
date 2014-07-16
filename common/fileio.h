/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

typedef struct fileio_s fileio_t;

fileio_t * fileio_open (const char *path);
int fileio_close (fileio_t *fio);
int fileio_seek_start (fileio_t *fio);
int fileio_seek_pos (fileio_t *fio, unsigned int pos);
char * fileio_get_string (fileio_t *fio, unsigned int *length);
char * fileio_read_string (fileio_t *fio, unsigned int *length);
unsigned int fileio_read_ui (fileio_t *fio);
double fileio_read_d (fileio_t *fio);
int fileio_eof (fileio_t *fio);
