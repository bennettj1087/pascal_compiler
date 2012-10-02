#ifndef POINTERLIST_H
#define POINTERLIST_H

#include "node.h"

typedef struct pointerlist_s {
	node_t *node;
	struct pointerlist_s *next;
} pointerlist_t;

pointerlist_t *make_ptrlist( node_t* );
pointerlist_t *add_ptr( pointerlist_t*, node_t* );
pointerlist_t *append_ptrlist( pointerlist_t*, pointerlist_t* );
void print_ptrlist( pointerlist_t* );

#endif
