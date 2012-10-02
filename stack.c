#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

void push(stack *s, char *str) {
  s->v[s->top]= strdup(str); 
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

char *stack_top(stack *s) {
  return s->v[s->top-1];
}

char *stack_swap(stack *s) {
  char *str1;
  char *str2;
  
  if (s->top == 1)
    return s->v[s->top-1];

  str1 = pop(s);
  str2 = pop(s);
  push(s, str1);
  push(s, str2);
  
  return s->v[s->top-1];
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


