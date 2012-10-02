#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "node.h"

/* make_node: constructor
 * copies input string name to the name field
 * next initialized to NULL
 */
node_t *make_node( char *name ) {
	node_t *p;
	p = (node_t *)malloc( sizeof(node_t) );
	assert( p != NULL );

	p->name = strdup(name);
	p->next = NULL;

	return p;
}

/* unmake_node: destructor
 * frees up string name and the input node
 */
void unmake_node( node_t *node ) {
	assert( node != NULL );
	if( node->name != NULL )
		free( node->name );
	free( node );
}

/* search: given a start node and a name,
 * decides if name is in the linked list defined by start
 */
node_t *search( node_t *start, char *name ) {
	node_t *p = start;

	while ( p != NULL ) {
		if ( !strcmp(p->name, name) )
			return p;
		p = p->next;
	}
	return NULL;
}

/* insert: given a start node and a name,
 * inserts name at the front of the linked list defined by start
 */
node_t *insert( node_t *start, char *name ) {
	node_t *p = make_node( name );

	if (start != NULL )
		p->next = start;
	return (p);
}

/* delete: given the start node,
 * deletes the entire linked list defined by start
 */
node_t *delete( node_t *start ) {
	node_t *p, *q;

	p = start;
	while ( p != NULL ) {
		q = p->next;
		unmake_node( p );
		p = q;
	}
	return NULL;
}
