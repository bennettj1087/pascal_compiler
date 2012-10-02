#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

void push(stack *s, char *str) {
  s->v[s->top] = strdup(str); 
  s->top++;    
}

char *pop(stack *s) {
  (s->top)--;
  return (s->v[s->top]);
}

stack *stack_init(stack *s) {
  s = calloc(1, sizeof(stack));
  s->top = 0;
  s->v = calloc(3, sizeof(char) * 32);

  return s;
}

int full(stack *s) {
  return (s->top >= 3);
}

char *swap_stack(stack *s) {
  char *str;
  strcpy(str, s->v[s->top]);
  strcpy(s->v[s->top], s->v[s->top-1]);
  strcpy(s->v[s->top-1], str);
  
  return s->v[s->top];
}

void stack_print(stack *s) {
  int i;
  if (s->top == 0)
    printf("Stack is empty.\n");
  else {
    printf("Stack contents: ");
    for (i = 0; i < s->top; i++) {
      printf("%s  ", s->v[i]); 
    }
    printf("\n");
  }
}

int main() {
  stack *registers;

  registers = stack_init(registers);
  push(registers, "test");
  push(registers, "more");
  stack_print(registers);
  swap_stack(registers);
  stack_print(registers);
}
