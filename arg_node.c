#include <stdio.h>
#include <stdlib.h>
#include "arg_node.h"

arg_node_t *make_arg() {
  arg_node_t *arg;
  arg = (arg_node_t *)malloc(sizeof(arg_node_t));
  arg->type = -1;
  arg->next = NULL;
  return arg;
}

arg_node_t *insert_arg(arg_node_t *list, arg_node_t *arg) {
  arg->next = list;
  return arg;
}

int count_arguments(arg_node_t *list) {
  int count = 0;
  while(list != NULL) {
	++count;
	list = list->next;
  }
  return count;
}
