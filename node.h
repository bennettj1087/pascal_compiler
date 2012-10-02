#ifndef NODE_H
#define NODE_H

#include "arg_node.h"

#define LOCAL 0
#define PARAMETER 1

typedef struct node_s {
  char *name;				// The identifier that corresponds to this variable
  int type;					// Either TYPE_INT, TYPE_REAL, or TYPE_NULL
  int is_function;
  int lower_bound;			// If this is not an array, both this and the next
  int upper_bound;			// values will be 0
  arg_node_t *arguments;		// If this is a function, this is a list of fn types
  struct node_s *next;			// Next in this linked list of nodes
  
  int offset;
  int local_or_parameter;
} node_t;

node_t	*make_node( char *name );
void	unmake_node( node_t *node );

node_t	*search( node_t *start, char *name );
node_t	*insert( node_t *start, char *name );
node_t	*delete( node_t *start );

#endif

