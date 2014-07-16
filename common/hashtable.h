/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

typedef struct hashtable_s hashtable_t;
typedef struct hashtable_node_s hashtable_node_t;
typedef struct hashtable_table_s hashtable_table_t;

#define HASHTABLE_PARAM_MASK 0xffff
#define HASHTABLE_PARAM_DEPTH 0x0

void * hashtable_add (hashtable_t *htable, char *name, void *data);
int hashtable_del (hashtable_t *htable, char *name);
void * hashtable_get_data (hashtable_t *htable, char *name);
hashtable_t * hashtable_init (unsigned int mask, unsigned int depth);
int hashtable_uninit (hashtable_t *htable);
