#include "tree.h"
#include "y.tab.h"
#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

tree_t *make_tree( int type, tree_t *left, tree_t *right ) {
	tree_t *p = (tree_t *)malloc(sizeof(tree_t));

	assert( p != NULL );
	p->type = type;
	p->left = left;
	p->right = right;

	return p;
}

tree_t *unmake_tree( tree_t *t) {
	if( t != NULL ) {
		unmake_tree( t->left );
		unmake_tree( t->right );
		free(t);
	}

	return (t = NULL);
}

tree_t *make_inum( int ival ) {
	tree_t *p = make_tree( INUM, NULL, NULL );
	p->attribute.ival = ival;
	p->actual_type = TYPE_INT;

	return p;
}

tree_t *make_rnum( float rval ) {
	tree_t *p = make_tree( RNUM, NULL, NULL );
	p->attribute.rval = rval;
	p->actual_type = TYPE_REAL;

	return p;
}

tree_t *make_ident( node_t *variable ) {
	tree_t *p = make_tree( ID, NULL, NULL );
	p->attribute.variable = variable;
	p->actual_type = variable->type;

	return p;
}

tree_t *make_op( int type, int opval, tree_t *left, tree_t *right ) {
	tree_t *p = make_tree( type, left, right );
	p->attribute.opval = opval;

	return p;
}

int check_arguments( tree_t *argtree, arg_node_t *typelist ) {
	arg_node_t **typelist_p = &typelist;
	if( !check_arguments_helper( argtree,  typelist_p ) ) return 0;
	return (typelist == NULL);
}

int check_arguments_helper( tree_t *argtree, arg_node_t **typelist ) {
	if(argtree->type == COMMA) {
		if(!check_arguments_helper(argtree->left, typelist)) return 0;
		return(check_arguments_helper(argtree->right, typelist));
	}
	else if((*typelist) == NULL) {
		return 0;
	}	
	else if(argtree->actual_type == (*typelist)->type) {
		(*typelist) = (*typelist)->next;
		return 1;
	}
	return 0;
}

void print_tree( tree_t *t, int spaces ) {
  if ( t == NULL) return;
  
  int i;
  
  // Process root
  for (i = 0; i < spaces; i++)
    fprintf( stderr, " ");
  switch(t->type) {
  case ID:
    fprintf( stderr, "[ID:%d:%i]", t->attribute.variable->type, 
	     t->attribute.variable->offset);//, t->attribute.variable->name );
    break;
  case INUM:
    fprintf( stderr, "[INUM:%d]", t->attribute.ival );
    break;
  case RNUM:
    fprintf( stderr, "[INUM:%f]", t->attribute.rval );
    break;
  case RELOP:
    fprintf( stderr, "[RELOP:%d]", t->attribute.opval );
    break;
  case ADDOP:
    fprintf( stderr, "[ADDOP:%d]", t->attribute.opval );
    break;
  case MULOP:
    fprintf( stderr, "[MULOP:%d]", t->attribute.opval );
    break;
  case COMMA:
    fprintf( stderr, "[COMMA]");
    break;
  case NOT:
    fprintf( stderr, "[NOT]");
    break;
  case ARRAY:
    fprintf( stderr, "[ARRAY]");
    break;
  case FUNCTION:
    fprintf( stderr, "[FUNCTION]");
    break;
  case PROCEDURE:
    fprintf( stderr, "[PROCEDURE: %s]", t->left->attribute.variable->name);
    break;
  case ASSIGNOP:
    fprintf( stderr, "[ASSIGNOP]");
    break;
  case IF:
    fprintf( stderr, "[IF]");
    break;
  case THEN:
    fprintf( stderr, "[THEN]");
    break;
  case BBEGIN:
    fprintf( stderr, "[BBEGIN]");
    break;
  case FOR:
    fprintf( stderr, "[FOR]");
    break;
  case TO:
    fprintf( stderr, "[TO]");
    break;
  default:
    fprintf( stderr, "[TYPE:%d]", t->type);
  }
  fprintf( stderr, "\n" );
  
  // Recurse to children
  print_tree( t->left, spaces+4 );
  print_tree( t->right, spaces+4 );
}

