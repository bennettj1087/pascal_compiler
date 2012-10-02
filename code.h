#ifndef CODE_H
#define CODE_H

#include "tree.h"
#include "pointerlist.h"

void gen_iostrings();
void gen_header();
void gen_footer();
void gen_realmain_header();
void gen_realmain_footer();
void gen_read(tree_t*);
void gen_write(tree_t*);
int gen_offsets(pointerlist_t*, int);

int gen_label(tree_t*, int);
void gen_expression(tree_t*);
int gen_expression_list(tree_t*); // Returns the number of expressions
void gen_assignment(tree_t*);
void gen_op(tree_t*, char*, char*);
void gen_statement(tree_t*);
char *find_variable(tree_t*);

#endif
