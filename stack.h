#ifndef _STACK_H
#define _STACK_H

typedef struct {
  char **v;
  int top;
} stack;

void push(stack *s, char *str);
char *pop(stack *s);
stack *stack_init(stack *s);
int full(stack *s);
char *stack_top(stack *s);
char *stack_swap(stack *s);
void stack_print(stack *s);

#endif
