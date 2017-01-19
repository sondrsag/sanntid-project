#ifndef _STACK_H_
#define _STACK_H_

#include "stdlib.h"
#include <stdbool.h>

typedef struct stack_node {
    struct stack_node* prev_node;
    int value;
    struct stack_node* next_node;
} stack_node_t;

typedef struct stack_root {
    stack_node_t* first_node;
    stack_node_t* last_node;
    unsigned int length;
} stack_root_t;

void stack_init(stack_root_t** stack);

// Add an integer (/node) to the top of the stack
void stack_push(stack_root_t* stack, int node);

// Remove and return the top node of the stack.
// For simplification and the application in this project we assume that
// negative node values are illegal, so -1 is returned when theres no nodes
// in the stack.
int stack_pop(stack_root_t* stack);

bool stack_is_empty(stack_root_t* stack);

#endif /* end of include guard: _STACK_H_ */
