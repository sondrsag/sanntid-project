#ifndef _STACK_H_
#define _STACK_H_

#include "stdlib.h"
#include <stdbool.h>

typedef struct stack_node {
    struct stack_node* prev_node;
    void* value;
    struct stack_node* next_node;
} stack_node_t;

typedef struct stack_root {
    stack_node_t* first_node;
    stack_node_t* last_node;
    unsigned int length;
} stack_root_t;

void stack_init(stack_root_t** stack);
void stack_destroy(stack_root_t** stack);

// Add an integer (/node) to the top of the stack
void stack_push(stack_root_t* stack, stack_node_t* node);

// Remove and return the top node of the stack
stack_node_t* stack_pop(stack_root_t* stack);

#endif /* end of include guard: _STACK_H_ */
