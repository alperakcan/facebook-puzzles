
/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

typedef struct stringtable_s stringtable_t;

int stringtable_uninit (stringtable_t *stringtable);
stringtable_t * stringtable_init (void);
void * stringtable_find (stringtable_t *stringtable, char *string);
void * stringtable_insert (stringtable_t *stringtable, char *string, void *data, int *already);
