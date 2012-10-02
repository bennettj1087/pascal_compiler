#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "pointerlist.h"
#include "types.h"

pointerlist_t *make_ptrlist( node_t *node ) {
	pointerlist_t *p;
	p = (pointerlist_t *)malloc( sizeof(pointerlist_t) );
	assert( p != NULL );

	p->node = node;
	p->next = NULL;

	return p;
}

pointerlist_t *add_ptr( pointerlist_t *list, node_t *node) {
	pointerlist_t *p = make_ptrlist( node );
	p->next = list;
	return p;
}

pointerlist_t *append_ptrlist( pointerlist_t *source, pointerlist_t *end) {
	pointerlist_t *iterator = source;
	
	while (iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = end;
	
	return source;
}

void print_ptrlist( pointerlist_t *p ) {
	while (p != NULL) {
		fprintf( stderr, "%s:%s ", p->node->name, type_string(p->node->type));
		p = p->next;
	}
	fprintf(stderr, "\n");
}

