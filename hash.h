#ifndef HASH_H
#define HASH_H

#include "node.h"

#define HASH_SIZE	211

typedef struct hash_s {
	int type;					/* FUNCTION or PROCEDURE, hopefully */
	node_t *curr;				/* the node corresponding to the current function or procedure */
	int returns;				/* the number of times this function or procedure returns */
	node_t *table[HASH_SIZE];	/* Weinberger's hash table */
	struct hash_s *next;		/* linkable */
}
hash_t;

hash_t	*make_hash(int type, node_t *curr);
void	unmake_hash( hash_t *table );
node_t	*hash_search( hash_t *start, char *name );
node_t	*hash_insert( hash_t *start, char *name );

hash_t	*hash_push( hash_t *start, int type, node_t *curr );
hash_t	*hash_pop( hash_t *top );
node_t	*hash_search_scopes( hash_t *top, char *name );
int		hash_find_depth( hash_t *top, char *name );

#endif

