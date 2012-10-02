#ifndef TREE_H
#define TREE_H

#include "node.h"
#include "arg_node.h"

typedef struct tree_s {
  int type;
  int actual_type;
  
  union {
    int ival;
    float rval;
    char *sval;
    int opval;
    node_t *variable;
  } attribute;
  
  struct tree_s *left;
  struct tree_s *right;

  int label;
} tree_t;

tree_t *make_tree( int type, tree_t *left, tree_t *right );
tree_t *unmake_tree( tree_t* );

tree_t *make_inum( int ival );
tree_t *make_rnum( float rval );
tree_t *make_ident( node_t *variable );
tree_t *make_op( int type, int opval, tree_t *left, tree_t *right );

int check_arguments( tree_t*, arg_node_t* );
int check_arguments_helper( tree_t*, arg_node_t** );

void print_tree( tree_t*, int );


#endif
