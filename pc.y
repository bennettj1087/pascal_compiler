%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "node.h"
#include "hash.h"
#include "tree.h"
#include "pointerlist.h"
#include "types.h"
#include "arg_node.h"
#include "code.h"
#include "stack.h"

hash_t *top_scope = NULL;
char buffer[50];
char *filename;
stack *registers;
stack *labels;
int curr_label;
int subprog_label;
int num_locals;
int true_label;
int if_label;
int while_label;
int for_label;

extern FILE *yyin;
extern FILE *yyout;
extern void yyerror(char*);
extern void yywarn(char*);
extern int yylex(void);

%}

%union {	/* attribute fields of token */
	int		ival;
	float	rval;
	char	*sval;
	int		opval;

	struct tree_s	*tval;
	struct pointerlist_s *plist;
	struct node_s	*node_info;
	struct arg_node_s *alist;
}

%token	PROGRAM VAR BBEGIN END
%token	ARRAY OF
%token	INTEGER REAL
%token	FUNCTION PROCEDURE
%token	IF THEN ELSE
%token	WHILE DO
%token	FOR TO

%token	DOTDOT
%token	ASSIGNOP

%token <opval>	RELOP
%token LT LE GT GE EQ NE

%token <opval>	ADDOP
%token PLUS MINUS OR

%token <opval>	MULOP
%token STAR SLASH MOD DIV AND

%token	NOT

%token	COMMA

%token <sval>	ID
%token <ival>	INUM
%token <rval>	RNUM

%type	<sval>	subprogram_head

%type	<tval>	optional_statements
%type	<tval>	statement_list
%type	<tval>	statement
%type	<tval>	matched_statement
%type	<tval>	unmatched_statement
%type	<tval>	compound_statement
%type	<tval>	procedure_statement

%type	<tval>	variable

%type	<tval>	expression_list
%type	<tval>	expression
%type	<tval>	simple_expression
%type	<tval>	term
%type	<tval>	factor

%type	<plist>	identifier_list
%type	<plist>	parameter_list
%type	<plist> arguments

%type	<node_info>	type
%type	<node_info> standard_type

%%

program
	: PROGRAM ID 
	{
		node_t *main_node = make_node($2);
		top_scope = hash_push( top_scope, PROCEDURE, main_node );
		
		// Insert "read" and "write" functions
		node_t *read_id = hash_insert( top_scope, "read" );
		read_id->type = TYPE_NULL;
		read_id->is_function = 1;
		read_id->lower_bound = 0;
		read_id->upper_bound = 0;
		arg_node_t *read_int = make_arg();
		read_int->type = TYPE_INT;
		read_id->arguments = read_int;

		node_t *write_id = hash_insert( top_scope, "write" );
		write_id->type = TYPE_NULL;
		write_id->is_function = 1;
		write_id->lower_bound = 0;
		write_id->upper_bound = 0;
		arg_node_t *write_int = make_arg();
		write_int->type = TYPE_INT;
		write_id->arguments = write_int;

		//Generate header 
		gen_header();
	}
	'(' identifier_list ')' ';'
	declarations
	subprogram_declarations
	{
		//Start realmain
		gen_realmain_header();

		//Make room on stack for locals		
		fprintf(yyout, "\tsubl\t$%d, %%esp\n", num_locals * 4 + 8);
	}
	compound_statement 
	'.'
	{
		print_tree($11, 0);

		//End realmain
		gen_realmain_footer();

		//Generate footer
		gen_footer();
	}
	;

identifier_list
	: ID
	{
		node_t *p = hash_insert( top_scope, $1 );
		if(p == NULL)
			yyerror("variable has already been declared in this scope");
		$$ = make_ptrlist( p );
	};
	| identifier_list ',' ID
	{
		node_t *p = hash_insert( top_scope, $3 );
		if(p == NULL)
			yyerror("variable has already been declared in this scope");
		$$ = add_ptr( $1, p );
	}
	;

declarations
	: declarations VAR identifier_list ':' type ';'
	{
		pointerlist_t *p;	
		p = $3;
		
		node_t *t;
		t = $5;

		// Set type information for each of the variables
		pointerlist_t *iter = p;
		while ( iter != NULL ) {
			iter->node->type = t->type;
			iter->node->lower_bound = t->lower_bound;
			iter->node->upper_bound = t->upper_bound;
			iter = iter->next;
		}

		// Generate offsets for assembly code and make room on stack
		num_locals = gen_offsets(p, LOCAL);
	}
	| /* empty */
	;

type
	: standard_type
	{ $$ = $1; }
	| ARRAY '[' INUM DOTDOT INUM ']' OF standard_type
	{
		node_t *p;
		p = $8;
		p->upper_bound = $5;
		p->lower_bound = $3;
		p->is_function = 0;
		$$ = p; 
	}
	;

standard_type
	: INTEGER
	{
		node_t *p;
		p = (node_t *)malloc( sizeof(node_t) );
		p->type = TYPE_INT;
		p->lower_bound = 0;
		p->upper_bound = 0;
		p->is_function = 0;
		$$ = p;
	}
	| REAL
	{
		node_t *p;
		p = (node_t *)malloc( sizeof(node_t) );
		p->type = TYPE_REAL;
		p->lower_bound = 0;
		p->upper_bound = 0;
		p->is_function = 0;
		$$ = p;
	}
	;

subprogram_declarations
	: subprogram_declarations subprogram_declaration ';'
	| /* empty */
	;

subprogram_declaration
	: subprogram_head declarations subprogram_declarations
	{
		// Generate the function header
  		fprintf(yyout, ".globl\tsubprog%s\n\t.type\tsubprog%s,@function\nsubprog%s:\n", $1, $1, $1);
 		fprintf(yyout, "\tpushl\t%%ebp\n\tmovl\t%%esp, %%ebp\n");
	}
	compound_statement
	{
		if(top_scope->type == FUNCTION && top_scope->returns < 1)
			yyerror("function must return a value");
		top_scope = hash_pop( top_scope );

		// Generate the function footer
		fprintf(yyout, "subprog%send:\n\tleave\n\tret\n", $1);

  		fprintf(yyout, ".Lfe%d:\n\t.size\tsubprog%s,.Lfe%d-subprog%s\n", subprog_label, $1, subprog_label, $1);
		++subprog_label;
	}
	;

subprogram_head
	: FUNCTION ID
	{
		// Add this name to the caller's scope
		node_t *identifier;
		identifier = hash_insert( top_scope, $2 );
		if( identifier == NULL ) {
			sprintf( buffer, "duplicate declaration of %s", $2 );
			yyerror( buffer );
		}

		// Push a new scope
		top_scope = hash_push( top_scope, FUNCTION, identifier );

	}
	arguments ':' standard_type ';'
	{
		node_t *t;
		t = $6;
		
		// Get the node for this function and set its type information
		node_t *identifier = hash_search_scopes( top_scope, $2 );
		identifier->type = t->type;
		identifier->lower_bound = t->lower_bound;
		identifier->upper_bound = t->upper_bound;
		identifier->is_function = 1;

		arg_node_t *args = NULL;
		pointerlist_t *p = $4;		

		// Compile a list of the argument types for semantic checking later
		pointerlist_t *iter = p;
		while(iter != NULL) {
			arg_node_t *n = make_arg();
			n->type = iter->node->type;
			args = insert_arg(args, n);
			iter = iter->next;
		}
		identifier->arguments = args;
		
		// Generate offsets for assembly code
		gen_offsets(p, PARAMETER);

		$$ = $2;
	}
	| PROCEDURE ID 
	{
		// Add this name to the caller's scope
		node_t *identifier;
		identifier = hash_insert( top_scope, $2 );
		if( identifier == NULL ) {
			sprintf( buffer, "duplicate declaration of %s", $2 );
			yyerror( buffer );
		}

		// Push a new scope
		top_scope = hash_push( top_scope, PROCEDURE, identifier );
	}
	arguments ';'
	{
		// Get the node for this procedure and set type information
		node_t *identifier = hash_search_scopes( top_scope, $2 );
		identifier->type = TYPE_NULL;
		identifier->lower_bound = 0;
		identifier->upper_bound = 0;
		identifier->arguments = NULL; 
		identifier->is_function = 1;

		arg_node_t *args = NULL;
		pointerlist_t *p = $4;		

		pointerlist_t *iter = p;
		while(iter != NULL) {
			arg_node_t *n = make_arg();
			n->type = iter->node->type;
			args = insert_arg(args, n);
			iter = iter->next;
		}
		identifier->arguments = args;

		// Generate offsets for assembly code
		gen_offsets(p, PARAMETER);

		$$ = $2;
	}
	;

arguments
	: '(' parameter_list ')'
	{ $$ = $2; }
	| /* empty */
	{ $$ = NULL; }
	;

parameter_list
	: identifier_list ':' type
	{
		pointerlist_t *p;
		p = $1;
		
		node_t *t;
		t = $3;

		pointerlist_t *iter = p;
		while ( iter != NULL ) {
			iter->node->type = t->type;
			iter->node->lower_bound = t->lower_bound;
			iter->node->upper_bound = t->upper_bound;
			iter = iter->next;
		}

		$$ = p;
	}
	| parameter_list ';' identifier_list ':' type
	{
		pointerlist_t *p;
		p = $3;
		
		node_t *t;
		t = $5;

		pointerlist_t *iter = p;
		while ( iter != NULL ) {
			iter->node->type = t->type;
			iter->node->lower_bound = t->lower_bound;
			iter->node->upper_bound = t->upper_bound;
			iter = iter->next;
		}

		pointerlist_t *o = $1;
		o = append_ptrlist( o, p );
		$$ = o;
	}
	;

compound_statement
	: BBEGIN optional_statements END
	{ $$ = $2; }
	;

optional_statements
	: statement_list
	{ $$ = $1; }
	| /* empty */
	{ $$ = NULL; }
	;

statement_list
	: statement
	{ 
		if ($1->type == IF || $1->type == WHILE ||
			$1->type == FOR || $1->type == ASSIGNOP || $1->type == PROCEDURE) {
			//print_tree($1, 0);	
			gen_statement($1);
		}
		$$ = $1; 
	}
	| statement_list ';' statement
	{ 
		if ($3->type == IF || $3->type == WHILE ||
			$3->type == FOR || $3->type == ASSIGNOP || $3->type == PROCEDURE) {
			//print_tree($3, 0);	
			gen_statement($3);
		}
		$$ = make_tree( COMMA, $1, $3 ); 
	}
	;

statement
	: matched_statement
	{ $$ = $1; }
	| unmatched_statement
	{ $$ = $1; }
	;

matched_statement
	: variable ASSIGNOP expression
	{ 
		// Ensure that variable is in this scope or that we are returning
		if($1->type == ID && hash_search( top_scope, $1->attribute.variable->name ) == NULL) {
			if($1->attribute.variable->name == top_scope->curr->name) {
				if(top_scope->type == FUNCTION) {
					top_scope->returns++;
				}
				else {
					sprintf( buffer, "return statement in procedure '%s'",
						top_scope->curr->name );
					yyerror( buffer );
				}
			}
			else if(top_scope->type == FUNCTION) {
				sprintf( buffer, "cannot assign global variable '%s' from function '%s'",
					$1->attribute.variable->name, top_scope->curr->name );
				yyerror( buffer );
			}
		}
		else if($1->type == ARRAY &&
					hash_search( top_scope, $1->left->attribute.variable->name ) == NULL &&
					top_scope->type == FUNCTION) {
			sprintf( buffer, "cannot assign index of global array '%s' from function '%s'",
				$1->left->attribute.variable->name, top_scope->curr->name );
			yyerror( buffer );
		}
		
		if ($1->actual_type != $3->actual_type) {
			sprintf(buffer, "type mismatch: tried to assign %s '%s' to %s",
				type_string($1->actual_type), $1->attribute.variable->name, type_string($3->actual_type));
			yyerror(buffer);
		}
		
		$$ = make_tree( ASSIGNOP, $1, $3 );
	}
	| procedure_statement
	{ $$ = $1; }
	| compound_statement
	{ $$ = $1; }
	| IF expression THEN matched_statement ELSE matched_statement 
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("if statement requires a boolean condition");

		$$ = make_tree( IF, $2, make_tree( THEN, $4, $6 ) );
	}
	| WHILE expression DO matched_statement
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("while statement requires a boolean condition");
		$$ = make_tree( WHILE, $2, $4 );
	}
	| FOR variable ASSIGNOP expression TO expression DO matched_statement
	{
		if($2->actual_type != $4->actual_type || $2->actual_type != $6->actual_type)
			yyerror("variable and expressions in for statement do not agree");
		$$ = make_tree( FOR, make_tree( TO, make_tree( ASSIGNOP, $2, $4 ), $6 ), $8 );
	}
	;

unmatched_statement
	: IF expression THEN statement
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("if statement requires a boolean condition");
		$$ = make_tree( IF, $2, $4 );
	}
	| IF expression THEN matched_statement ELSE unmatched_statement
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("if statement requires a boolean condition");
		$$ = make_tree( IF, $2, make_tree( THEN, $4, $6 ) );
	}
	| WHILE expression DO unmatched_statement
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("while statement requires a boolean condition");
		$$ = make_tree( WHILE, $2, $4 );
	}
	| FOR variable ASSIGNOP expression TO expression DO unmatched_statement
	{
		if($2->actual_type != $4->actual_type || $2->actual_type != $6->actual_type)
			yyerror("variable and expressions in for statement do not agree");
		$$ = make_tree( FOR, make_tree( TO, make_tree( ASSIGNOP, $2, $4 ), $6 ), $8 );
	}
	;

variable
	: ID
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1 );
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else
			$$ = make_ident( identifier );
	}
	| ID '[' expression ']'
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1 );
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else {
			tree_t *t;
			t = make_tree( ARRAY, make_ident(identifier), $3 );
			t->actual_type = identifier->type;
			$$ = t;			
			if ( $3->actual_type != TYPE_INT ) {
				sprintf( buffer, "used non-integer to reference array '%s'", $1 );
				yyerror( buffer );
			}
			else if ( identifier->lower_bound == 0 && identifier->upper_bound == 0 ) {
				sprintf(buffer, "'%s' is not an array", $1);
				yyerror(buffer);
			}
		}
	}
	;

procedure_statement
	: ID
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1);
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else if ( identifier->type != TYPE_NULL || identifier->lower_bound != 0 ||
				  identifier->upper_bound != 0 || identifier->is_function == 0 ) {
			sprintf( buffer, "'%s' is not a procedure, but is being used like one", $1 );
			yyerror( buffer );
		}
		else if ( identifier->arguments != NULL ) {
			sprintf( buffer, "procedure '%s' requires arguments", $1 );
			yyerror( buffer );
		}
		else {	
			$$ = make_tree( PROCEDURE, make_ident(identifier), NULL );			
		}
	}
	| ID '(' expression_list ')'
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1);
		if( identifier == NULL ) {
			sprintf(buffer, "'%s' is undefined", $1);
			yyerror(buffer);
		}
		else if ( identifier->type != TYPE_NULL || identifier->lower_bound != 0 ||
				  identifier->upper_bound != 0 || identifier->is_function == 0 ) {
			sprintf( buffer, "'%s' is not a procedure", $1 );
			yyerror( buffer );
		}
		else if ( !check_arguments($3, identifier->arguments) ) { 
			sprintf( buffer, "arguments don't match the procedure '%s'", $1 );
			yyerror( buffer );
		}
		else {
			$$ = make_tree( PROCEDURE, make_ident(identifier), $3 );
		}
	}
	;

expression_list
	: expression
	{ 
		tree_t *t = $1;
		gen_label(t, 1);
		$$ = t;	
	}
	| expression_list ',' expression
	{	 
		tree_t *t = $3;
		gen_label(t, 1);
		$$ = make_tree( COMMA, $1, $3 ); 
	}
	;

expression
	: simple_expression
	{ 
		$$ = $1; 
	}
	| simple_expression RELOP simple_expression
	{ 
		tree_t *t;
		t = make_op( RELOP, $2, $1, $3 );
		if ($1->actual_type != $3->actual_type) {
			sprintf(buffer, "type mismatch: tried to compare %s to %s",
				type_string($1->actual_type), type_string($3->actual_type));
			yyerror(buffer);
		}
		else
			t->actual_type = TYPE_BOOL;
		$$ = t;
	}
	;

simple_expression
	: term
	{ $$ = $1; }
	| ADDOP term
	{
		tree_t *t;
		t = make_op( ADDOP, $1, $2, NULL );
		t->actual_type = $2->actual_type;
	}
	| simple_expression ADDOP term
	{ 
		tree_t *t;
		// Basic optimization: if $1 and $3 are both INUMs, then make this node an INUM as the answer.
		if ($1->type == INUM && $3->type == INUM) {
			if($2 == PLUS)
				t = make_inum( $1->attribute.ival + $3->attribute.ival );
			else
				t = make_inum( $1->attribute.ival - $3->attribute.ival );
		}
		else {
			t = make_op( ADDOP, $2, $1, $3 );
			if ($2 != OR && $1->actual_type != $3->actual_type) {
				if($2 == PLUS)
					sprintf(buffer, "type mismatch: tried to add %s to %s",
						type_string($1->actual_type), type_string($3->actual_type));
				if($2 == MINUS)
					sprintf(buffer, "type mismatch: tried to subtract %s from %s",
						type_string($1->actual_type), type_string($3->actual_type));
				yyerror(buffer);
			}
			else if ($2 == OR && ($1->actual_type != TYPE_BOOL || $3->actual_type != TYPE_BOOL))
				yyerror("type mismatch: expected boolean values in OR statement");
			else
				t->actual_type = $1->actual_type;
		}
		$$ = t;
	}
	;

term
	: factor
	{ $$ = $1; }
	| term MULOP factor
	{ 
		tree_t *t;
		// Basic optimization: if $1 and $3 are both INUMs, then make this node an INUM as the answer.
		if ($1->type == INUM && $3->type == INUM) {
			if($2 == STAR)
				t = make_inum( $1->attribute.ival * $3->attribute.ival );
			else
				t = make_inum( $1->attribute.ival / $3->attribute.ival );
		}
		else {
			t = make_op( MULOP, $2, $1, $3 );
			if ($2 != AND && $1->actual_type != $3->actual_type) {
				if($2 == STAR)
					sprintf(buffer, "type mismatch: tried to multiply %s with %s",
						type_string($1->actual_type), type_string($3->actual_type));
				if($2 == SLASH)
					sprintf(buffer, "type mismatch: tried to divide %s with %s",
						type_string($1->actual_type), type_string($3->actual_type));
				if($2 == DIV)
					sprintf(buffer, "type mismatch: tried to div %s with %s",
						type_string($1->actual_type), type_string($3->actual_type));
				if($2 == MOD)
					sprintf(buffer, "type mismatch: tried to mod %s with %s",
						type_string($1->actual_type), type_string($3->actual_type));
				yyerror(buffer);
			}
			else if ($2 == AND && ($1->actual_type != TYPE_BOOL || $3->actual_type != TYPE_BOOL))
				yyerror("type mismatch: expected boolean values in AND statement");
			else
				t->actual_type = $1->actual_type;
		}

		$$ = t;
	}
	;

factor
	: ID
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1 );
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else if ( identifier->type == TYPE_NULL ) {
			sprintf(buffer, "'%s' should be a function or variable", $1);
			yyerror( buffer );
		}
		else if ( identifier->arguments != NULL ) {
			sprintf( buffer, "function '%s' requires arguments", $1 );
			yyerror( buffer );
		}
		else if ( identifier->upper_bound != 0 || identifier->lower_bound != 0 ) {
			sprintf( buffer, "array '%s' requires an index", $1 );
			yyerror( buffer );
		}
		else
			$$ = make_ident( identifier );
	}
	| ID '(' expression_list ')'
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1 );
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else if ( identifier->type == TYPE_NULL || identifier->upper_bound != 0 ||
				identifier->lower_bound != 0 || identifier->is_function == 0 ) {
			sprintf(buffer, "'%s' is not a function", $1);
			yyerror( buffer );
		}
		else if ( !check_arguments($3, identifier->arguments) ) {
			sprintf(buffer, "argument mismatch in function '%s'", $1);
			yyerror( buffer );
		}
		else {
			tree_t *t;
			t = make_tree( FUNCTION, make_ident(identifier), $3 );
			t->actual_type = identifier->type;
	
			$$ = t;
		}
	}
	| ID '[' expression ']'
	{
		node_t *identifier;
		identifier = hash_search_scopes( top_scope, $1 );
		if( identifier == NULL ) {
			sprintf(buffer, "name '%s' undefined", $1);
			yyerror(buffer);
		}
		else if ( identifier->lower_bound == 0 && identifier->upper_bound == 0 ) {
			sprintf(buffer, "'%s' is not an array", $1);
			yyerror(buffer);
		}
		else if ( $3->actual_type != TYPE_INT ) {
			sprintf( buffer, "used non-integer to reference array '%s'", $1 );
			yyerror( buffer );
		}
		else {
			tree_t *t;
			t = make_tree( ARRAY, make_ident(identifier), $3 );
			t->actual_type = identifier->type;
			$$ = t;
		}
	}
	| INUM
	{ $$ = make_inum( $1 ); }
	| RNUM
	{ $$ = make_rnum( $1 ); }
	| '(' expression ')'
	{ $$ = $2; }
	| NOT factor
	{
		if ( $2->actual_type != TYPE_BOOL ) yyerror("'not' requires a boolean condition");
		tree_t *t;
		t = make_tree( NOT, $2, NULL );
		t->actual_type = $2->actual_type;
		$$ = t;
	}
	;

%%



int main( int argc, char *argv[] ) {
	char *str = calloc(2, sizeof(char));	

	if( argc < 2 ) {
		fprintf( stderr, "Usage: mypc <input_file>\n" );
		exit(1);
	}

	filename = strdup(argv[1]);
	yyin = fopen( argv[1], "r");

	str = strrchr(argv[1], '.');
	strcpy(str, ".s");

	yyout = fopen( argv[1], "w");

	//Initialize label stack
	labels = stack_init(labels);
	curr_label = 0;
	subprog_label = 3;
	true_label = 0;
	if_label = 0;
	while_label = 0;
	for_label = 0;

	//Initialize register stack
	registers = stack_init(registers);
	push(registers, "%esi");
	push(registers, "%edi");
	push(registers, "%ebx");

	yyparse();

	fclose(yyin);
	fclose(yyout);
	
	return 0;
}
