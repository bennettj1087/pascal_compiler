#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hash.h"

#define EOS '\0'

/* hashpjw: Peter J. Weinberger's "compiler" hash function */
int hashpjw( char *s ) {
	char *p;
	unsigned h = 0, g;
	
	for ( p = s; *p != EOS; p++ ) {
		h = (h << 4) + *p;
		if ( (g = h & 0xf0000000) ) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	
	return h % HASH_SIZE;
}

hash_t *make_hash(int type, node_t *curr) {
	int i;
	hash_t *p;

	p = (hash_t *)malloc( sizeof(hash_t) );
	assert( p != NULL );

	for ( i = 0; i < HASH_SIZE; i++ )
		p->table[i] = NULL;
	p->next = NULL;
	p->type = type;
	p->curr = curr;
	p->returns = 0;

	return p;
}

void unmake_hash( hash_t *t ) {
	int i;

	if( t != NULL ) {
		for (i = 0; i < HASH_SIZE; i++ )
			delete( t->table[i] );
		t->next = NULL;
		free( t );
	}
}

node_t *hash_search( hash_t *start, char *name ) {
	int index = hashpjw( name );
	return search( start->table[index], name );
}

node_t *hash_insert( hash_t *start, char *name ) {
	
	int index = hashpjw( name );
	node_t *p = start->table[index];

	if( search( p, name ) != NULL ) return NULL;
	else return start->table[index] = insert(p, name);
}

hash_t *hash_push( hash_t *top, int type, node_t *curr ) {
	hash_t *p = make_hash(type, curr);

	p->next = top;
	return p;
}

hash_t *hash_pop( hash_t *top ) {
	hash_t *p;

	if( top != NULL ) {
		p = top;
		top = top->next;
		unmake_hash( p );
	}

	return top;
}

node_t *hash_search_scopes( hash_t *start, char *name ) {
	node_t *p;
	hash_t *scope;

	scope = start;
	while( scope != NULL ) {
		if( (p = hash_search( scope, name )) != NULL ) return p;
		scope = scope->next;
	}

	return NULL;
}

int hash_find_depth( hash_t *start, char *name ) {
	node_t *p;
	hash_t *scope;
	int depth = 0;

	scope = start;
	while( scope != NULL ) {
		if( (p = hash_search( scope, name )) != NULL ) return depth;
		scope = scope->next;
		++depth;
	}

	return -1;
}

