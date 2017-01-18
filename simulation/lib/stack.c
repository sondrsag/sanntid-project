#include "stack.h"
#include <stdio.h>

void stack_init(stack_root_t** stack)
{
    *stack = malloc(sizeof(stack_root_t));
    (*stack)->length = 0;
}

void stack_destroy(stack_root_t** stack)
{
    free(*stack);
}

void stack_push(stack_root_t* stack, stack_node_t* node)
{
    printf("stack push\n");
    if (stack->length > 0) {
        stack->last_node->next_node = node;
        stack->last_node = node;
        stack->length++;
        node->next_node = NULL;
    } else {
        stack->first_node = node;
        stack->last_node = node;
        stack->length++;
        node->next_node = NULL;
    }
}

stack_node_t* stack_pop(stack_root_t* stack)
{
    printf("stack pop\n");
    if (stack->length == 0) return NULL;

    stack_node_t* return_node = stack->last_node;

    if (stack->length == 1) {
        stack->last_node = NULL;
        stack->first_node = NULL;
        stack->length--;
        return return_node;
    }

    stack->last_node = stack->first_node;

    while (stack->last_node->next_node != return_node) {
        stack->last_node = stack->last_node->next_node;
    }

    stack->last_node->next_node = NULL;
    return return_node;
}

int main(int argc, char const *argv[]) {
    stack_root_t* stack;
    stack_init(&stack);
    stack_node_t node1;
    stack_node_t node2;
    node1.value = (int*) 1;
    node2.value = (int*) 2;
    stack_push(stack, &node1);
    stack_push(stack, &node2);
    int* ret = stack_pop(stack)->value;
    printf("%d\n", *ret);
    stack_destroy(&stack);
    return 0;
}
