#ifndef ARG_NODE_H
#define ARG_NODE_H

typedef struct arg_node_s {
  int type;
  struct arg_node_s *next;
} arg_node_t;

arg_node_t *make_arg();
arg_node_t *insert_arg(arg_node_t *list, arg_node_t *arg);
int count_arguments(arg_node_t *list);

#endif
