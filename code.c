#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
#include "code.h"
#include "tree.h"
#include "stack.h"
#include "hash.h"

#define LOCAL_OFFSET 12
#define PARAM_OFFSET 8
#define NUM_REG 3

extern FILE *yyout;
extern void yywarn(char*);
extern void yyerror(char*);
extern char buffer[50];
extern char *filename;
extern stack *registers;
extern int true_label;
extern int if_label;
extern int while_label;
extern int for_label;
extern hash_t *top_scope;

char buff1[10];
char buff2[10];

void gen_iostrings() {
  fprintf(yyout, ".LC0:\n\t.string\t\"%%d\"\n");
  fprintf(yyout, ".LC1:\n\t.string\t\"%%d\\n\"\n\t.text\n");
}

void gen_header() {
  fprintf(yyout, "\t.file\t\"%s\"\n", filename);
  fprintf(yyout, "\t.text\n");
  fprintf(yyout, ".globl main\n");
  fprintf(yyout, "\t.type\tmain,@function\n");
  fprintf(yyout, "main:\n");
  fprintf(yyout, "\tpushl\t%%ebp\n");
  fprintf(yyout, "\tmovl\t%%esp, %%ebp\n");
  fprintf(yyout, "\tsubl\t$8, %%esp\n");
  fprintf(yyout, "\tandl\t$-16, %%esp\n");
  fprintf(yyout, "\tmovl\t$0, %%eax\n");
  fprintf(yyout, "\tsubl\t%%eax, %%esp\n");
  fprintf(yyout, "\tcall\trealmain\n");
  fprintf(yyout, "\tleave\n");
  fprintf(yyout, "\tret\n");
  fprintf(yyout, ".Lfe1:\n");
  fprintf(yyout, "\t.size\tmain,.Lfe1-main\n");
  
  // Set strings for printf and scanf
  gen_iostrings();
}

void gen_footer() {
  fprintf(yyout, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
  fprintf(yyout, "\t.ident\t\"GCC: (GNU) 3.2.3 20030502 (Red Hat Linux 3.2.3-59)\"\n");
}

void gen_realmain_header() {
  // Start realmain function where everything will go
  fprintf(yyout, ".globl\trealmain\n\t.type\trealmain,@function\nrealmain:\n");
  fprintf(yyout, "\tpushl\t%%ebp\n\tmovl\t%%esp, %%ebp\n");  
}

void gen_realmain_footer() {
  fprintf(yyout, "\tleave\n\tret\n");
  fprintf(yyout, ".Lfe2:\n\t.size\trealmain,.Lfe2-realmain\n");
}

void gen_read(tree_t *t) {
  fprintf(yyout, "\tleal\t-%i(%%ebp), %%eax\n", t->attribute.variable->offset);
  fprintf(yyout, "\tpushl\t%%eax\n");
  fprintf(yyout, "\tpushl\t$.LC0\n");
  fprintf(yyout, "\tcall\tscanf\n");
  fprintf(yyout, "\taddl\t$8, %%esp\n");
}

void gen_write(tree_t *t) {
  if ( t->type == ID ) {
    if (t->attribute.variable->local_or_parameter == LOCAL)
      fprintf(yyout, "\tpushl\t-%i(%%ebp)\n\tpushl\t$.LC1\n\tcall\tprintf\n\taddl\t$8, %%esp\n", 
	      t->attribute.variable->offset);
    else
      fprintf(yyout, "\tpushl\t%i(%%ebp)\n\tpushl\t$.LC1\n\tcall\tprintf\n\taddl\t$8, %%esp\n", 
	      t->attribute.variable->offset);
  }
  else if ( t->type == INUM )
    fprintf(yyout, "\tpushl\t$%i\n\tpushl\t$.LC1\n\tcall\tprintf\n\taddl\t$8, %%esp\n", 
	    t->attribute.ival);
  else
    fprintf(yyout, "\tpushl\t%%eax\n\tpushl\t$.LC1\n\tcall\tprintf\n\taddl\t$8, %%esp\n");
}

int gen_offsets(pointerlist_t *p, int local_or_parameter) {
  int c_offset;
  int count = 0;

  if (local_or_parameter == LOCAL)
    c_offset = LOCAL_OFFSET;
  else
    c_offset = PARAM_OFFSET;

  while (p != NULL) {
    p->node->local_or_parameter = local_or_parameter;
    p->node->offset = c_offset;

    count++;
    c_offset += 4;
    p = p->next;
  }
  
  return count;
}

int gen_label(tree_t *t, int left) {
  int r_label, l_label;

  if ( t->left == NULL && t->right == NULL ) {
    if (left == 1)
      t->label = 1;
    else
      t->label = 0;
  }
  else {
    l_label = gen_label(t->left, 1);
    r_label = gen_label(t->right, 0);

    if (l_label > r_label)
      t->label = l_label;
    else if (r_label > l_label)
      t->label = r_label;
    else
      t->label = r_label + 1;
  }
  
  return t->label;
}

void gen_expression(tree_t *t) {
  char *reg;

  // If the tree is actually a function, then take care of it here.
  if (t->type == FUNCTION) {
    // Push the arguments on the stack and call the function
    int num_args = gen_expression_list(t->right);
    fprintf(yyout, "\tcall\tsubprog%s\n", t->left->attribute.variable->name);

    // Move the top of the stack back down below the arguments
    fprintf(yyout, "\taddl\t$%d, %%esp\n", 4*num_args);

    // Move the answer to the top of the register stack
    fprintf(yyout, "\tmovl\t%%eax, %s\n", stack_top(registers));
  }

  // Case 0:
  else if (t->right == NULL && t->left == NULL && t->label == 1) {
    if (t->type == ID ) {
	fprintf(yyout, "\tmovl\t%s, %s\n", find_variable(t), stack_top(registers));
    }
    else if (t->type == INUM) {
      fprintf(yyout, "\tmovl\t$%i, %s\n", t->attribute.ival, 
	      stack_top(registers));
    }
    else {
      yywarn("Type not supported");
    }
  }
  else {
    // Case 1:
    if (t->right->label == 0) {
      // Go to left
      gen_expression(t->left);

      if (t->right->type == ID) {
	  gen_op(t, find_variable(t->right), stack_top(registers));
      }
      else if (t->right->type == INUM) {
        sprintf(buff1, "$%i", t->right->attribute.ival);
	  gen_op(t, buff1, stack_top(registers));
      }
      else {
	  yywarn("Type not supported");
      }
    }
    // Case 2:
    else if (t->left->label >= 1 && t->right->label > t->left->label && t->left->label < NUM_REG) {
      stack_swap(registers);
      gen_expression(t->right);
      reg = pop(registers);
      gen_expression(t->left);
      
      gen_op(t, reg, stack_top(registers));

   	push(registers, reg);
	stack_swap(registers);
    }
    // Case 3:
    else if (t->right->label >= 1 && t->left->label >= t->right->label && t->right->label < NUM_REG) {
	gen_expression(t->left);
      reg = pop(registers);
      gen_expression(t->right);
      
      gen_op(t, stack_top(registers), reg);
      push(registers, reg);
    }
    // Case 4:
    else {
      yywarn("Temporaries not yet implemented");
    }
  }
}


int gen_expression_list(tree_t *t) {
  if(t->type == COMMA) {
    return gen_expression_list(t->left) + gen_expression_list(t->right);
  }
  else {
    gen_label(t, 1);	
    gen_expression(t);
    fprintf(yyout, "\tpushl\t%s\n", stack_top(registers));
    return 1;
  }
}

void gen_assignment(tree_t *t) {
	fprintf(yyout, "\tmovl\t%%eax, %s\n", find_variable(t));
}

void gen_op(tree_t *t, char *op1, char *op2) {
  // Output operation
  switch(t->attribute.opval) {
  case OR:
  case PLUS:
    fprintf(yyout, "\taddl\t%s, %s\n", op1, op2);
    break;
  case MINUS:
    fprintf(yyout, "\tsubl\t%s, %s\n", op1, op2);
    break;
  case AND:
  case STAR:
    fprintf(yyout, "\timull\t%s, %s\n", op1, op2);
    break;
  case SLASH:
  case DIV:
    fprintf(yyout, "\tmovl\t%s, %%eax\n\tmovl\t%%eax, -4(%%ebp)\n\tmovl\t%s, %%eax\n\tcltd\n", op1, op2);
    fprintf(yyout, "\tidivl\t-4(%%ebp)\nmovl\t%%eax, %s\n", op2);
    break;
  case MOD:
    fprintf(yyout, "\tmovl\t%s, %%eax\n\tmovl\t%%eax, -4(%%ebp)\n\tmovl\t%s, %%eax\n\tcltd\n", op1, op2);
    fprintf(yyout, "\tidivl\t-4(%%ebp)\n\tmovl\t%%edx, %s\n", op2);
    break;
  case LT:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tjl\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  case GT:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tjg\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  case LE:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tjle\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  case GE:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tjge\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  case NE:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tjne\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  case EQ:
    fprintf(yyout, "\tcmpl\t%s, %s\n\tje\tsettrue%d\n", op1, op2, true_label);
    fprintf(yyout, "\tmovl\t$0, %s\n\tjmp\tsetfalse%d\nsettrue%d:\n\tmovl\t$1, %s\nsetfalse%d:\n", op2, true_label, true_label, op2, true_label);
    true_label++;
    break;
  default:
    yywarn("operations not yet supported");
    break;
  }
}

void gen_statement(tree_t *t) {
  int local_if_label;
  int local_while_label;
  int local_for_label;
  int num_args;

  if (t->type == ARRAY || t->right->type == ARRAY || t->left->type == ARRAY) {
    yyerror("Array access not supported");
  }

  switch(t->type) {
  case ASSIGNOP:
    // Generate code for expression
    gen_label(t->right, 1);
    gen_expression(t->right);
    fprintf(yyout, "\tmovl\t%%ebx, %%eax\n");
    
    // If this is actually a return statement, then return with correct value in %eax
    if(!strcmp(t->left->attribute.variable->name, top_scope->curr->name)) 
      fprintf(yyout, "\tjmp\tsubprog%send\n", top_scope->curr->name);
    else {
      // Generate code for assignment
      gen_assignment(t->left);
    }

    break;
  case IF:
    local_if_label = if_label++;
    
    //Generate code for expression
    gen_label(t->left, 1);
    gen_expression(t->left);

    //Compare and jump accordingly
    fprintf(yyout, "\tcmpl\t$1, %%ebx\n\tjge\tiftrue%d\n", local_if_label);

    //Generate code for else first
    if (t->right->right != NULL) {
      gen_statement(t->right->right);
      fprintf(yyout, "\tjmp\tifend%d\n", local_if_label);
    }
    
    //Generate code for true section
    fprintf(yyout, "iftrue%d:\n", local_if_label);
    gen_statement(t->right->left);
    fprintf(yyout, "ifend%d:\n", local_if_label);

    break;
  case WHILE:
    local_while_label = while_label++;

    //Plant label for beginning of loop
    fprintf(yyout, "beginwhile%d:\n", local_while_label);
    
    //Generate code for expression
    gen_label(t->left, 1);
    gen_expression(t->left);

    //Compare and jump to end if needed
    fprintf(yyout, "\tcmpl\t$0, %%ebx\n\tje\tendwhile%d\n", local_while_label);

    //Generate code for statements
    gen_statement(t->right);
    fprintf(yyout, "\tjmp\tbeginwhile%d\n", local_while_label);
    fprintf(yyout, "endwhile%d:\n", local_while_label);

    break;
  case PROCEDURE:
    if (!strcmp(t->left->attribute.variable->name, "read"))
      gen_read(t->right);
    else if (!strcmp(t->left->attribute.variable->name, "write"))
      gen_write(t->right);
    else if (t->right != NULL) {
      // push arguments on the stack
      num_args = gen_expression_list(t->right); 
      fprintf(yyout, "\tcall\tsubprog%s\n", t->left->attribute.variable->name);

      // move the top of the stack below args
      fprintf(yyout, "\taddl\t$%d, %%esp\n", 4*num_args); 
    }
    else {
      fprintf(yyout, "\tcall\tsubprog%s\n", t->left->attribute.variable->name);
    }
    
    break;
  case COMMA:
    // Generate left and right statements
    if (t->left->type == ARRAY || t->right->type == ARRAY)
      yyerror("Array access not supported");
    else {
      gen_statement(t->left);
      gen_statement(t->right);
    }
    break;
  case FOR:
    local_for_label = for_label++;

    // Generate assignment
    gen_statement(t->left->left);

    //Plant label for beginning
    fprintf(yyout, "forloop%d:\n", local_for_label);

    // Generate expression for TO value
    gen_label(t->left->right, 1);
    gen_expression(t->left->right);

    //Compare and jump to end if needed
    if (t->left->left->left->attribute.variable->local_or_parameter == LOCAL)
      fprintf(yyout, "\tcmpl\t-%i(%%ebp), %%ebx\n\tjl\tforend%d\n",
	      t->left->left->left->attribute.variable->offset, local_for_label);
    else
      fprintf(yyout, "\tcmpl\t-%i(%%ebp), %%ebx\n\tjl\tforend%d\n",
	      t->left->left->left->attribute.variable->offset, local_for_label); 

    //Generate code for statements
    gen_statement(t->right);

    //Increment, jump to top then plant end label
    fprintf(yyout, "\tincl\t-%i(%%ebp)\n\tjmp\tforloop%d\nforend%d:\n", 
	    t->left->left->left->attribute.variable->offset,
	    local_for_label, local_for_label);

    break;
  default:
    fprintf(stderr, "TYPE: %d\n", t->type);
    yywarn("Statement not supported");
    break;
  }
}

char *find_variable(tree_t *t) {
   int depth = hash_find_depth(top_scope, t->attribute.variable->name);
   if(depth < 1) {
      if (t->attribute.variable->local_or_parameter == LOCAL)
	   sprintf(buff1, "-%i(%%ebp)", t->attribute.variable->offset);
	else
	   sprintf(buff1, "%i(%%ebp)", t->attribute.variable->offset);
   }
   else {
	fprintf(yyout, "\tmovl\t%%ebp, %%ecx\n");
	while(depth > 0) {
	   fprintf(yyout, "\tmovl\t(%%ecx), %%ecx\n");
	   --depth;
	}
	if (t->attribute.variable->local_or_parameter == LOCAL)
	   sprintf(buff1, "-%i(%%ecx)", t->attribute.variable->offset);
	else
	   sprintf(buff1, "%i(%%ecx)", t->attribute.variable->offset);
   }
   return buff1;
}
