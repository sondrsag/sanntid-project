#ifndef _STACK_H_
#define _STACK_H_

#include "stdlib.h"
#include <stdbool.h>

typedef struct stack_node {
    int value;
    struct stack_node* next_node;
} stack_node_t;

typedef struct stack_root {
    stack_node_t* first_node;
    unsigned int length;
} stack_root_t;

void stackInit(stack_root_t** stack);

// Add an integer (/node) to the top of the stack
void stackPush(stack_root_t* stack, int node);

// Remove and return the top node of the stack.
// For simplification and the application in this project we assume that
// negative node values are illegal, so -1 is returned when theres no nodes
// in the stack.
int stackPop(stack_root_t* stack);

bool stackIsEmpty(stack_root_t* stack);

#endif /* end of include guard: _STACK_H_ */